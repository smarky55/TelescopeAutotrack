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
#include "webserver.hpp"

#include "TMCStepper.h"
#include "uart.hpp"
#include "uartStream.hpp"

#define MILLIS 1000
#define SECONDS 1000000

constexpr gpio_num_t BLINK_GPIO = GPIO_NUM_13;

static const char* TAG = "telescope";

static void uart_printer(void*) {
  UART::Config uartConf;
  uartConf.txPin = 17;
  uartConf.rxPin = 16;
  uartConf.baud = 9600;
  uartConf.stopBits = UART::StopBits::Bits_1;

  UART::UARTDriver driver(UART::Port::Port1, uartConf);
  while (1) {
    if (driver.available()) {
      ESP_LOGI(TAG, "Echo: %x", driver.receiveByte());
    } else {
      vTaskDelay(50);
    }
  }
}

void my_main(void) {
  /*gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

  Queue<StepperMessage> queue(10);
  TaskHandle_t task;
  StepperController controller(&queue);

  controller.setDirection(0);
  controller.setSpeed(100);
  controller.setAccelleration(100 * 32);
  controller.setJerk(20);

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

  wifi_init_sta();

  httpd_handle_t server = nullptr;
  ESP_ERROR_CHECK(esp_event_handler_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &webserverConnectHandler, &server));
  ESP_ERROR_CHECK(
      esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                 &webserverDisconnectHandler, &server));

  server = webserverStart();

  webserverSetCommandCallback([&controller](httpd_req_t* request) {
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

    cJSON* button = cJSON_GetObjectItem(json, "button");
    if (!button) {
      return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
    }
    ESP_LOGI(TAG, "Button %s", button->valuestring);
    if (strcmp(button->valuestring, "left") == 0) {
      controller.setDirection(-1);
      controller.setSpeed(1 * 200 * 32);
    } else if (strcmp(button->valuestring, "track") == 0) {
      controller.setDirection(1);
      controller.setSpeed(21.3*0.5);
    } else if (strcmp(button->valuestring, "right") == 0) {
      controller.setDirection(1);
      controller.setSpeed(1 * 200 * 32);
    } else if (strcmp(button->valuestring, "stop") == 0) {
      controller.setDirection(0);
      controller.setSpeed(0);
    } else {
      return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, NULL);
    }

    return httpd_resp_send(request, nullptr, 0);
  });

  xTaskCreatePinnedToCore(&stepperTask, "stepper", 4096, &queue,
                          tskIDLE_PRIORITY + 1, &task, 1);
  */

  UART::Config uartConf;
  uartConf.txPin = 15;
  uartConf.rxPin = 33;
  uartConf.baud = 9600;
  uartConf.stopBits = UART::StopBits::Bits_1;

  UART::UARTDriver driver(UART::Port::Port2, uartConf);
  UART::UARTStream stream(&driver);
  TMC2209Stepper stepper(&stream, 0.11, 0);

  xTaskCreate(uart_printer, "uart_echo", 2048, nullptr, 10, nullptr);
  vTaskDelay(100);
  stepper.defaults();
  stepper.begin();
  stepper.microsteps(256);
  stepper.rms_current(980, 0.1f);
  stepper.TPOWERDOWN(20);
  stepper.TPWMTHRS(0);
  stepper.freewheel(1);
  ESP_LOGI(TAG, "TMC2209 version: %d", stepper.version());
  vTaskDelay(200);
  int VACTUAL = 71583;

  float rampUp[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
  float rampDown[] = {0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};

  while (1) {
    for (int i : rampUp) {
      stepper.VACTUAL(VACTUAL * i);
      vTaskDelay(100);
    }
    stepper.VACTUAL(VACTUAL);
    vTaskDelay(5000);
    for (int i : rampDown) {
      stepper.VACTUAL(VACTUAL * i);
      vTaskDelay(100);
    }
    stepper.VACTUAL(0);
    vTaskDelay(5000);
    for (int i : rampUp) {
      stepper.VACTUAL(-VACTUAL * i);
      vTaskDelay(100);
    }
    stepper.VACTUAL(-VACTUAL);
    vTaskDelay(5000);
    for (int i : rampDown) {
      stepper.VACTUAL(-VACTUAL * i);
      vTaskDelay(100);
    }
  }
}

extern "C" {
void app_main(void) {
  my_main();
}
}