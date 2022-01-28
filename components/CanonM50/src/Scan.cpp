#include "CanonM50/Scan.hpp"

#include <NimBLEDevice.h>

#include <esp_log.h>

static const char* TAG = "CanonM50 - Scan";

Scan::Scan(const NimBLEUUID& serviceUUID)
    : m_wantedUUID(serviceUUID), m_mutex(xSemaphoreCreateMutex()) {}

bool Scan::start(uint32_t duration) {
  using namespace std::placeholders;

  m_devices.clear();

  ESP_LOGI(TAG, "Starting BLE scan. Duration: %d", duration);
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(this);
  scan->setActiveScan(true);
  return scan->start(duration, std::bind(&Scan::onScanComplete, this, _1));
}

bool Scan::stop() {
  NimBLEScan* scan = NimBLEDevice::getScan();
  ESP_LOGI(TAG, "Stopping BLE scan");
  if (scan->stop()) {
    m_scanning = false;
    return true;
  }
  return false;
}

bool Scan::isScanning() const {
  return m_scanning;
}

void Scan::onResult(NimBLEAdvertisedDevice* device) {
  if (device->isAdvertisingService(m_wantedUUID)) {
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    m_devices.push_back(device);
    xSemaphoreGive(m_mutex);
  }
}

std::vector<NimBLEAdvertisedDevice*> Scan::devices() const {
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  auto temp = m_devices;
  xSemaphoreGive(m_mutex);
  return temp;
}

void Scan::onScanComplete(NimBLEScanResults) {
  ESP_LOGI(TAG, "BLE scan complete. Found %d matching devices.",
           m_devices.size());
  m_scanning = false;
}
