#include "uartStream.hpp"

#include "uart.hpp"

using namespace UART;

UARTStream::UARTStream(UARTDriver* driver) : m_driver(driver) {}

int UARTStream::available() {
  return m_driver->available();
}

uint8_t UARTStream::write(const uint8_t data) {
  m_driver->sendByte(data);
}

uint8_t UARTStream::read() {
  return m_driver->receiveByte();
}