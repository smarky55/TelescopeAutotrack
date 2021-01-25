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

#define MILLIS 1000
#define SECONDS 1000000

constexpr gpio_num_t BLINK_GPIO = GPIO_NUM_13;

static const char* TAG = "telescope";

void my_main(void) {
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

  Queue<StepperMessage> queue(10);
  TaskHandle_t task;
  StepperController controller(&queue);

  controller.setDirection(0);
  controller.setSpeed(1 * 200 * 32);
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


  xTaskCreatePinnedToCore(&stepperTask, "stepper", 4096, &queue,
                          tskIDLE_PRIORITY + 1, &task, 1);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

extern "C" {
void app_main(void) {
  my_main();
}
}