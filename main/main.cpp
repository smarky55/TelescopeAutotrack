#include <cJSON.h>
#include <esp_netif.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "Queue.hpp"
#include "smk_wifi.hpp"
#include "stepper.hpp"
#include "telescope.hpp"
#include "webserver.hpp"

#include "TMCStepper.h"
#include "uart.hpp"
#include "uartStream.hpp"

#include <CanonM50/CanonM50.hpp>
#include <CanonM50/Scan.hpp>

#define MILLIS 1000
#define SECONDS 1000000

constexpr gpio_num_t BLINK_GPIO = GPIO_NUM_13;

static const char* TAG = "telescope";

// static void uart_printer(void*) {
//   UART::Config uartConf;
//   uartConf.txPin = 17;
//   uartConf.rxPin = 16;
//   uartConf.baud = 9600;
//   uartConf.stopBits = UART::StopBits::Bits_1;

//   UART::UARTDriver driver(UART::Port::Port1, uartConf);
//   while (1) {
//     if (driver.available()) {
//       ESP_LOGI(TAG, "Echo: %x", driver.receiveByte());
//     } else {
//       vTaskDelay(50);
//     }
//   }
// }

static void tmcStepperTask(void* arg) {
  Telescope* scope = static_cast<Telescope*>(arg);
  scope->tick();  // Get the first dud out the way
  while (1) {
    vTaskDelay(100);
    scope->tick();
  }
}

void my_main(void) {
  vTaskDelay(pdMS_TO_TICKS(50));

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  vTaskDelay(pdMS_TO_TICKS(50));

  wifi_init_softap();

  httpd_handle_t server = nullptr;
  ESP_ERROR_CHECK(esp_event_handler_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &webserverConnectHandler, &server));
  ESP_ERROR_CHECK(
      esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                 &webserverDisconnectHandler, &server));

  server = webserverStart();

  // xTaskCreatePinnedToCore(&stepperTask, "stepper", 4096, &queue,
  //                         tskIDLE_PRIORITY + 1, &task, 1);

  UART::Config uartConf;
  uartConf.txPin = 32;
  uartConf.rxPin = 33;
  uartConf.baud = 9600;
  uartConf.stopBits = UART::StopBits::Bits_1;

  UART::UARTDriver driver(UART::Port::Port2, uartConf);
  UART::UARTStream stream(&driver);
  TMC2209Stepper stepper(&stream, 0.11, 0);

  CanonM50 camera("ESP32-Telescope");
  camera.init();

  // xTaskCreate(uart_printer, "uart_echo", 2048, nullptr, 10, nullptr);
  vTaskDelay(100);
  stepper.defaults();
  stepper.begin();
  stepper.microsteps(256);
  stepper.rms_current(800, 0.1f);
  stepper.TPOWERDOWN(20);
  stepper.TPWMTHRS(0);
  stepper.freewheel(1);
  ESP_LOGI(TAG, "TMC2209 version: %d", stepper.version());

  stepper.VACTUAL(0);
  vTaskDelay(1000);
  stepper.VACTUAL(10000);
  vTaskDelay(2000);
  stepper.VACTUAL(0);
  vTaskDelay(2000);

  Telescope scope(&stepper, 1);
  xTaskCreate(tmcStepperTask, "telescopeTask", 2048, &scope, 10, nullptr);

  webserverSetCommandCallback([&scope, &camera](httpd_req_t* request) {
    char buf[100];
    int ret, remaining = request->content_len;

    if (remaining > sizeof(buf)) {
      return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
    }

    ret = httpd_req_recv(request, buf, remaining);
    if (ret < 0) {
      return ESP_FAIL;
    }

    cJSON* json = cJSON_Parse(buf);
    if (!json) {
      return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
    }

    cJSON* command = cJSON_GetObjectItem(json, "command");
    if (!command) {
      cJSON_Delete(json);
      return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
    }
    ESP_LOGI(TAG, "Command %s", command->valuestring);
    if (strcmp(command->valuestring, "track") == 0) {
      scope.setTracking(true);
    } else if (strcmp(command->valuestring, "stop") == 0) {
      scope.setTracking(false);
      scope.setTargetRASpeed(0);
    } else if (strcmp(command->valuestring, "speed") == 0) {
      cJSON* value = cJSON_GetObjectItem(json, "value");
      if (!value) {
        cJSON_Delete(json);
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
      }
      scope.setTargetRASpeed(value->valueint * 0.01);  // Range -1 <-> 1
    } else if (strcmp(command->valuestring, "update") == 0) {
      cJSON* trackRate = cJSON_GetObjectItem(json, "trackRate");
      if (!trackRate) {
        cJSON_Delete(json);
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
      }
      scope.setTrackRate(trackRate->valueint);
    } else if (strcmp(command->valuestring, "cam-scan") == 0) {
      camera.getScan()->start(5);
    } else if (strcmp(command->valuestring, "cam-list") == 0) {
      httpd_resp_set_type(request, HTTPD_TYPE_JSON);
      cJSON* resp = cJSON_CreateObject();
      cJSON_AddBoolToObject(resp, "scanning", camera.getScan()->isScanning());
      cJSON* items = cJSON_AddArrayToObject(resp, "items");
      for (auto* device : camera.getScan()->devices()) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "address",
                                device->getAddress().toString().c_str());
        cJSON_AddStringToObject(item, "name", device->getName().c_str());
        cJSON_AddStringToObject(item, "manufacturer",
                                device->getManufacturerData().c_str());
        cJSON_AddItemToArray(items, item);
      }
      std::string respStr(cJSON_PrintUnformatted(resp));
      cJSON_Delete(resp);
      cJSON_Delete(json);
      return httpd_resp_send(request, respStr.c_str(), HTTPD_RESP_USE_STRLEN);
    } else if (strcmp(command->valuestring, "cam-connect") == 0) {
      cJSON* address = cJSON_GetObjectItem(json, "address");
      if (!address) {
        cJSON_Delete(json);
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
      }
      camera.pair(std::string(address->valuestring));
    } else if (strcmp(command->valuestring, "cam-trigger") == 0) {
      camera.trigger();
    } else {
      cJSON_Delete(json);
      return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
    }

    cJSON_Delete(json);
    return httpd_resp_send(request, nullptr, 0);
  });

  while (1) {
    vTaskDelay(5000);
  }
}

extern "C" {
void app_main(void) {
  my_main();
}
}