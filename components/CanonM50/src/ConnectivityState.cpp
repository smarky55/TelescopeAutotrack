#include "CanonM50/ConnectivityState.hpp"

#include <esp_log.h>

static const char* TAG = "CanonM50 - Conn";

bool ConnectivityState::isConnected() const {
  return m_connected;
}

void ConnectivityState::onConnect(NimBLEClient* client) {
  ESP_LOGI(TAG, "Connected to device: %s",
           client->getPeerAddress().toString().c_str());
  m_connected = true;
}

void ConnectivityState::onDisconnect(NimBLEClient* client) {
  ESP_LOGI(TAG, "Disconnected from device: %s",
           client->getPeerAddress().toString().c_str());
  m_connected = false;
}

bool ConnectivityState::onConfirmPIN(uint32_t pin) {
  ESP_LOGI(TAG, "Pin confirm %d", pin);
  vTaskDelay(pdMS_TO_TICKS(5000));
  return true;
}

uint32_t ConnectivityState::onPassKeyRequest() {
  return 123456;
}

void ConnectivityState::onAuthenticationComplete(ble_gap_conn_desc* desc) {}
