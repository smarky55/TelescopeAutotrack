#include "uartStream.hpp"

#include "uart.hpp"

#include <cstdio>
#include <esp_log.h>

static const char* TAG = "UART";

using namespace UART;

UARTStream::UARTStream(UARTDriver* driver) : m_driver(driver) {}

int UARTStream::available() {
  return m_driver->available()-m_ignoreBytes;
}

uint8_t UARTStream::write(const uint8_t data) {
  uint8_t ret = m_driver->sendByte(data);
  ++m_ignoreBytes;
  return ret;
}

int16_t UARTStream::read() {
  while (m_ignoreBytes > 0) {
    --m_ignoreBytes;
    m_driver->receiveByte();
  }
  int16_t ret = m_driver->receiveByte();
  return ret;
}