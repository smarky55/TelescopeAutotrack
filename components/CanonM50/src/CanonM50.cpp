#include "CanonM50/CanonM50.hpp"

#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEUUID.h>

#include <esp_log.h>
#include <nvs.h>

static const char* TAG = "CanonM50";

static const NimBLEUUID SERVICE_UUID("00050000-0000-1000-0000-d8492fffa821");
static const NimBLEUUID PAIRING_UUID("00050002-0000-1000-0000-d8492fffa821");
static const NimBLEUUID SHUTTER_UUID("00050003-0000-1000-0000-d8492fffa821");

static const uint8_t BUTTON_RELEASE = 0b10000000;
static const uint8_t BUTTON_FOCUS   = 0b01000000;
static const uint8_t BUTTON_TELE    = 0b00100000;
static const uint8_t BUTTON_WIDE    = 0b00010000;
static const uint8_t MODE_IMMEDIATE = 0b00001100;
static const uint8_t MODE_DELAY     = 0b00000100;
static const uint8_t MODE_MOVIE     = 0b00001000;

static const char* NVS_NS = "CanonM50";
static const char* NVS_CAMERA_ADDRESS = "lastCameraAddr";

CanonM50::CanonM50(const std::string& name)
    : m_client(NimBLEDevice::createClient()),
      m_lastDeviceAddress(NimBLEAddress("")),
      m_name(name),
      m_scan(SERVICE_UUID) {
  m_client->setClientCallbacks(&m_connectionState);
}

CanonM50::~CanonM50() = default;

void CanonM50::init() {
  NimBLEDevice::init(m_name);

  ESP_ERROR_CHECK(nvs_open(NVS_NS, NVS_READWRITE, &m_nvs));

  size_t len;
  esp_err_t ret = nvs_get_str(m_nvs, NVS_CAMERA_ADDRESS, NULL, &len);
  if (ret == ESP_OK) {
    std::string addr;
    addr.reserve(len);
    ESP_ERROR_CHECK(nvs_get_str(m_nvs, NVS_CAMERA_ADDRESS, addr.data(), &len));

    m_lastDeviceAddress = NimBLEAddress(addr);
  } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
    // Not finding the key is OK
    ESP_LOGI(TAG, "No previous device found");
  } else {
    ESP_ERROR_CHECK(ret);
  }
}

bool CanonM50::pair(const NimBLEAddress& address) {
  ESP_LOGD(TAG, "Connecting to device %s", address.toString().c_str());
  if (!m_client->connect(address)) {
    ESP_LOGE(TAG, "Failed to connect to client %s", address.toString().c_str());
    return false;
  }
  ESP_LOGD(TAG, "Successfully connected to device");

  m_remoteService = m_client->getService(SERVICE_UUID);
  if (m_remoteService == nullptr) {
    ESP_LOGE(TAG, "Failed to find expected service on device %s",
             address.toString().c_str());
    return false;
  }
  ESP_LOGD(TAG, "Got remote service: %s", m_remoteService->toString().c_str());

  m_pairingCharacteristic = m_remoteService->getCharacteristic(PAIRING_UUID);
  if (m_pairingCharacteristic == nullptr) {
    ESP_LOGE(TAG, "Failed to get pairing characteristic on device %s",
             address.toString().c_str());
    return false;
  }

  std::vector<uint8_t> cmd{{0x03}};
  cmd.insert(cmd.end(), m_name.begin(), m_name.end());
  if (!m_pairingCharacteristic->writeValue(cmd.data(), cmd.size(), false)) {
    ESP_LOGE(TAG, "Failed to write pairing command");
    return false;
  }

  m_shutterCharacteristic = m_remoteService->getCharacteristic(SHUTTER_UUID);
  if (m_shutterCharacteristic == nullptr) {
    ESP_LOGE(TAG, "Failed to get shutter characteristic on device %s",
             address.toString().c_str());
    return false;
  }

  m_lastDeviceAddress = address;
  ESP_LOGD(TAG, "Saving address to nvs");
  ESP_ERROR_CHECK(nvs_set_str(m_nvs, NVS_CAMERA_ADDRESS,
                              std::string(m_lastDeviceAddress).c_str()));
  ESP_ERROR_CHECK(nvs_commit(m_nvs));

  return true;
}

bool CanonM50::connect() {
  ESP_LOGD(TAG, "Connecting to device %s",
           m_lastDeviceAddress.toString().c_str());
  if (!m_client->connect(m_lastDeviceAddress)) {
    ESP_LOGE(TAG, "Failed to connect to client %s",
             m_lastDeviceAddress.toString().c_str());
    return false;
  }
  ESP_LOGD(TAG, "Successfully connected to device");

  m_remoteService = m_client->getService(SERVICE_UUID);
  if (m_remoteService == nullptr) {
    ESP_LOGE(TAG, "Failed to find expected service on device %s",
             m_lastDeviceAddress.toString().c_str());
    m_client->disconnect();
    return false;
  }
  ESP_LOGD(TAG, "Got remote service: %s", m_remoteService->toString().c_str());

  m_shutterCharacteristic = m_remoteService->getCharacteristic(SHUTTER_UUID);
  if (m_shutterCharacteristic == nullptr) {
    ESP_LOGE(TAG, "Failed to get shutter characteristic on device %s",
             m_lastDeviceAddress.toString().c_str());
    m_client->disconnect();
    return false;
  }
  return true;
}

bool CanonM50::disconnect() {
  return m_client->disconnect() == 0;
}

bool CanonM50::isConnected() const {
  return m_connectionState.isConnected();
}

bool CanonM50::trigger() {
  if (!isConnected() && !connect()) {
    ESP_LOGE(TAG, "Cannot trigger, unable to connect to device.");
    return false;
  }

  uint8_t cmd = MODE_IMMEDIATE | BUTTON_RELEASE;
  if (!m_shutterCharacteristic->writeValue(cmd)) {
    ESP_LOGE(TAG, "Failed to send trigger cmd");
    return false;
  }
  vTaskDelay(pdMS_TO_TICKS(200));
  if (!m_shutterCharacteristic->writeValue(MODE_IMMEDIATE)) {
    ESP_LOGE(TAG, "Failed to send trigger release cmd");
    return false;
  }
  return true;
}

Scan* CanonM50::getScan() {
  return &m_scan;
}
