#include "stepper.hpp"

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <cmath>

#include "Queue.hpp"

static const char* TAG = "Stepper";

static const gpio_num_t DIR_PIN = GPIO_NUM_14;
static const gpio_num_t STP_PIN = GPIO_NUM_32;
static const gpio_num_t SLP_PIN = GPIO_NUM_15;

static void stepperTimerCallback(void* arg) {
  // Wake task
  xTaskNotifyGive(arg);
}

void stepperTask(void* params) {
  auto messageQueue = static_cast<Queue<StepperMessage>*>(params);
  bool level = false;
  TaskHandle_t myHandle = xTaskGetCurrentTaskHandle();

  const esp_timer_create_args_t timerArgs = {.callback = &stepperTimerCallback,
                                             .arg = myHandle,
                                             .dispatch_method = ESP_TIMER_TASK,
                                             .name = "StepperTimer"};
  esp_timer_handle_t stepperTimer;
  ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &stepperTimer));

  StepperState state;
  uint64_t lastTime = esp_timer_get_time();
  ESP_ERROR_CHECK(esp_timer_start_once(stepperTimer, state.getNextTimeout(0)));
  uint64_t stepTime = lastTime + state.getNextTimeout(0);
  bool dirPin = false;
  bool sleepPin = true;

  gpio_reset_pin(DIR_PIN);
  gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);
  gpio_reset_pin(SLP_PIN);
  gpio_set_direction(SLP_PIN, GPIO_MODE_OUTPUT);
  gpio_reset_pin(STP_PIN);
  gpio_set_direction(STP_PIN, GPIO_MODE_OUTPUT);

  gpio_set_level(SLP_PIN, sleepPin);

  for (;;) {
    while (messageQueue->messagesWaiting()) {
      auto message = messageQueue->receive(0);
      if (!message.has_value())
        break;
      switch (message->inst) {
        case StepperInstruction::SetSpeed:
          state.setSpeed(message->fVal);
          break;
        case StepperInstruction::SetAccelleration:
          state.setAccelleration(message->fVal);
          break;
        case StepperInstruction::SetDirection:
          state.setDirection(message->iVal);
          break;

        default:
          break;
      }
    }

    bool didstep = false;

    // Wait for the timer to expire
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
    uint64_t currentTime = esp_timer_get_time();
    if (currentTime >= stepTime) {
      if (dirPin != state.direction()) {
        dirPin = !dirPin;
        gpio_set_level(DIR_PIN, dirPin);
      }

      if (sleepPin == state.sleep()) {
        sleepPin = !sleepPin;
        gpio_set_level(SLP_PIN, sleepPin);
      } else {
        gpio_set_level(STP_PIN, level);
        level = !level;
      }
      didstep = true;
    } else {
      // Timer hasn't triggered yet
      ESP_ERROR_CHECK(esp_timer_stop(stepperTimer));
    }

    // Calculate when the next step should take place.
    stepTime = lastTime + state.getNextTimeout(currentTime - lastTime);
    ESP_ERROR_CHECK(esp_timer_start_once(stepperTimer, stepTime - currentTime));

    if (didstep) {
      // Update last step time to now
      lastTime = currentTime;
    }
  }

  vTaskDelete(NULL);
}

void StepperState::setSpeed(float speed) {
  m_targetSpeed = speed;
}

void StepperState::setAccelleration(float acc) {
  m_accelleration = acc;
}

void StepperState::setDirection(int32_t dir) {
  if (dir < 0) {
    m_direction = -1;
  } else if (dir > 0) {
    m_direction = 1;
  } else {
    m_direction = 0;
  }
}

int8_t StepperState::direction() const {
  return m_currentVelocity > 0;
}

void StepperState::setJerk(float jerk) {
  m_jerk = jerk;
}

uint64_t StepperState::getNextTimeout(uint64_t deltaT) {
  float deltaS = m_accelleration * deltaT * (1 / 1000000.0);

  float targetV = m_targetSpeed * m_direction;

  // ESP_LOGI(TAG, "deltaT: %lld deltaS: %f target: %f current: %f", deltaT,
  //          deltaS, targetV, m_currentVelocity);

  if (fabs(m_currentVelocity - targetV) < m_jerk) {
    m_currentVelocity = targetV;
  } else if (m_currentVelocity > targetV) {
    m_currentVelocity -= deltaS;
    if (m_currentVelocity < targetV) {
      m_currentVelocity = targetV;
    }
  } else if (m_currentVelocity < targetV) {
    m_currentVelocity += deltaS;
    if (m_currentVelocity > targetV) {
      m_currentVelocity = targetV;
    }
  }

  if (fabs(m_currentVelocity) < 0.1f) {
    return 10000000;
  } else {
    return m_currentVelocity < 0 ? 1000000ULL / -m_currentVelocity
                                 : 1000000ULL / m_currentVelocity;
  }
}

bool StepperState::sleep() const {
  return m_direction == 0 && fabs(m_currentVelocity) < 0.001;
}

StepperController::StepperController(Queue<StepperMessage>* messageQueue)
    : m_queue(messageQueue) {}

void StepperController::setSpeed(float speed) {
  m_queue->send({StepperInstruction::SetSpeed, speed}, 0);
}

void StepperController::setAccelleration(float acc) {
  m_queue->send({StepperInstruction::SetAccelleration, acc}, 0);
}

void StepperController::setDirection(int8_t dir) {
  m_queue->send({StepperInstruction::SetDirection, dir}, 0);
}

void StepperController::setJerk(float jerk) {
  m_queue->send({StepperInstruction::SetJerk, jerk}, 0);
}