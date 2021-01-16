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

static void periodicTimerCallback(void* arg) {
  int64_t timeSinceBoot = esp_timer_get_time();
  ESP_LOGI(TAG, "Periodic timer called, time since boot: %lld us",
           timeSinceBoot);
  // StepperMessage message = {StepperInstruction::SetSpeed, 10};
  // static_cast<Queue<StepperMessage>*>(arg)->send(message, 0);
}

void my_main(void) {
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

  Queue<StepperMessage> queue(10);
  TaskHandle_t task;
  xTaskCreate(&stepperTask, "stepper", 2048, &queue, tskIDLE_PRIORITY + 1,
              &task);
  StepperController controller(&queue);

  const esp_timer_create_args_t periodicTimerArgs = {
      .callback = &periodicTimerCallback,
      .arg = &queue,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "periodic"};

  esp_timer_handle_t periodicTimer;
  ESP_ERROR_CHECK(esp_timer_create(&periodicTimerArgs, &periodicTimer));

  // ESP_ERROR_CHECK(esp_timer_start_periodic(periodicTimer, 2 * SECONDS));
  controller.setDirection(1);
  controller.setSpeed(10);
  controller.setAccelleration(0.1);
  controller.setJerk(1);
  vTaskDelay(2500 / portTICK_PERIOD_MS);

  while (1) {
    // printf("Turning off the LED\n");
    // gpio_set_level(BLINK_GPIO, 0);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // printf("Turning on the LED\n");
    // gpio_set_level(BLINK_GPIO, 1);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    controller.setDirection(-1);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    controller.setDirection(1);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

extern "C" {
void app_main(void) {
  my_main();
}
}