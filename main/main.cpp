#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "Queue.hpp"
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
  xTaskCreate(&stepperTask, "stepper", 2048, &queue, tskIDLE_PRIORITY + 1,
              &task);
  StepperController controller(&queue);
;
  controller.setDirection(0);
  controller.setSpeed(5*200*32);
  controller.setAccelleration(100*32);
  controller.setJerk(20);

  while (1) {
    controller.setDirection(1);
    vTaskDelay(20000 / portTICK_PERIOD_MS);

    controller.setDirection(-1);
    vTaskDelay(20000 / portTICK_PERIOD_MS);
  }
}

extern "C" {
void app_main(void) {
  my_main();
}
}