#include "CanonM50/IntervalometerTask.hpp"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "CanonM50 - Interval";

void IntervalometerTask::s_intervalometerTask(void* arg) {
  auto p_this = static_cast<IntervalometerTask*>(arg);
  while (1) {
    p_this->doTask();
  }
}

IntervalometerTask::IntervalometerTask()
    : m_taskHandle(nullptr),
      m_camera(nullptr),
      m_waitMs(1000),
      m_count(0) {}

IntervalometerTask::~IntervalometerTask() {
  end();
}

bool IntervalometerTask::begin(uint32_t stackDepth, uint32_t priority) {
  if (m_taskHandle) {
    return false;
  }
  BaseType_t status = xTaskCreate(s_intervalometerTask, "intervalometer",
                                  stackDepth, this, priority, &m_taskHandle);
  return status == pdPASS;
}

void IntervalometerTask::end() {
  if (m_taskHandle) {
    vTaskDelete(m_taskHandle);
    m_taskHandle = nullptr;
  }

  m_count = 0;
}

void IntervalometerTask::runSequence(CanonM50* camera,
                                     float imageDuration,
                                     float interval,
                                     int count) {
  if (m_count > 0) {
    abortSequence();
  }

  m_camera = camera;
  m_waitMs = imageDuration + interval;
  m_count = count;
  ESP_LOGI(TAG, "Start sequence Wait: %fms, Count: %d", float(m_waitMs), int(m_count));
}

void IntervalometerTask::abortSequence() {
  if (m_count <= 0) {
    return;
  }
  ESP_LOGI(TAG, "Abort sequence");
  m_count = 0;
}

bool IntervalometerTask::inSequence() const {
  return m_count > 0;
}

int IntervalometerTask::remainingCount() const {
  return m_count;
}

void IntervalometerTask::doTask() {
  TickType_t lastWakeTime = xTaskGetTickCount();
  if (m_count > 0) {
    ESP_LOGI(TAG, "Sequence trigger");
    // Trigger the camera
    m_camera->trigger();

    // Decrement remaining count
    m_count--;

    // If there are more images remaining in sequence, sleep until 
    if (m_count > 0) {
      ESP_LOGI(TAG, "Waiting %d remaining", int(m_count));
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(m_waitMs));
    }
  } else {
    vTaskDelay(100);
  }
}
