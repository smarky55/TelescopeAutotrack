#include "uart.hpp"

#include <esp_log.h>
#include <driver/uart.h>
#include <soc/uart_periph.h>

using namespace UART;

UARTDriver::UARTDriver(Port port, const Config& config) : m_port(port) {
  setBaudRate(config.baud);
  setParity(config.parity);
  setStopBits(config.stopBits);
  setFlowControl(config.flowControl, config.flowCtrlRxThresh);

  ESP_ERROR_CHECK(uart_set_word_length(getImplPort(), UART_DATA_8_BITS));

  setPins(config.txPin, config.rxPin, config.rtsPin, config.ctsPin);

  install(config.txBufferSize, config.rxBufferSize);
  // setHalfDuplex();
}

UARTDriver::~UARTDriver() {
  remove();
}

void UARTDriver::setHalfDuplex() {
  uart_set_mode(getImplPort(), UART_MODE_RS485_APP_CTRL);
  UART2.rs485_conf.rx_busy_tx_en = false;
  UART2.rs485_conf.tx_rx_en = true;
}

void UARTDriver::setBaudRate(int baud) {
  ESP_ERROR_CHECK(uart_set_baudrate(getImplPort(), baud));
}

void UARTDriver::setParity(Parity parity) {
  switch (parity) {
    case Parity::Disable:
      ESP_ERROR_CHECK(uart_set_parity(getImplPort(), UART_PARITY_DISABLE));
      break;
    case Parity::Even:
      ESP_ERROR_CHECK(uart_set_parity(getImplPort(), UART_PARITY_EVEN));
      break;
    case Parity::Odd:
      ESP_ERROR_CHECK(uart_set_parity(getImplPort(), UART_PARITY_ODD));
      break;
  }
}

void UARTDriver::setStopBits(StopBits stopBits) {
  switch (stopBits) {
    case StopBits::Bits_1:
      ESP_ERROR_CHECK(uart_set_stop_bits(getImplPort(), UART_STOP_BITS_1));
      break;
    case StopBits::Bits_1_5:
      ESP_ERROR_CHECK(uart_set_stop_bits(getImplPort(), UART_STOP_BITS_1_5));
      break;
    case StopBits::Bits_2:
      ESP_ERROR_CHECK(uart_set_stop_bits(getImplPort(), UART_STOP_BITS_2));
      break;
  }
}

void UARTDriver::setFlowControl(FlowControl ctrl, int rxThresh) {
  switch (ctrl) {
    case FlowControl::Disable:
      ESP_ERROR_CHECK(
          uart_set_hw_flow_ctrl(getImplPort(), UART_HW_FLOWCTRL_DISABLE, 0));
      break;
    case FlowControl::CTS:
      ESP_ERROR_CHECK(
          uart_set_hw_flow_ctrl(getImplPort(), UART_HW_FLOWCTRL_CTS, 0));
      break;
    case FlowControl::RTS:
      ESP_ERROR_CHECK(
          uart_set_hw_flow_ctrl(getImplPort(), UART_HW_FLOWCTRL_RTS, rxThresh));
      break;
    case FlowControl::Both:
      ESP_ERROR_CHECK(uart_set_hw_flow_ctrl(
          getImplPort(), UART_HW_FLOWCTRL_CTS_RTS, rxThresh));
      break;
  }
}

void UARTDriver::setPins(int txPin, int rxPin, int rtsPin, int ctsPin) {
  ESP_ERROR_CHECK(uart_set_pin(getImplPort(), txPin, rxPin, rtsPin, ctsPin));
}

void UARTDriver::install(int txBufferSize, int rxBufferSize) {
  ESP_ERROR_CHECK(uart_driver_install(getImplPort(), rxBufferSize, txBufferSize,
                                      0, nullptr, 0));
}

void UARTDriver::remove() {
  if (uart_is_driver_installed(getImplPort())) {
    ESP_ERROR_CHECK(uart_driver_delete(getImplPort()));
  }
}

size_t UARTDriver::available() {
  size_t ret = 0;
  ESP_ERROR_CHECK(uart_get_buffered_data_len(getImplPort(), &ret));
  return ret;
}

size_t UARTDriver::sendByte(uint8_t data) {
  int ret = uart_write_bytes(getImplPort(), (char*)&data, 1);
  return ret;
}

size_t UARTDriver::sendBytes(const std::vector<uint8_t>& data) {
  return uart_write_bytes(getImplPort(), reinterpret_cast<const char*>(data.data()),
                          data.size());
}

int UARTDriver::receiveByte(uint32_t timeout /*= 0*/) {
  uint8_t ret;
  if (uart_read_bytes(getImplPort(), &ret, 1, timeout) <= 0) {
    return -1;
  }
  return ret;
}

int  UARTDriver::receiveBytes(std::vector<uint8_t>& data,
                              uint32_t timeout /*= 0*/) {
  size_t length = 0;
  ESP_ERROR_CHECK(uart_get_buffered_data_len(getImplPort(), &length));
  data.resize(length);
  return uart_read_bytes(getImplPort(), data.data(), length, timeout);
}

void UARTDriver::flushRxBuffer() {
  ESP_ERROR_CHECK(uart_flush(getImplPort()));
}

constexpr int UARTDriver::getImplPort() const {
  switch (m_port) {
    case Port::Port0:
      return UART_NUM_0;
    case Port::Port1:
      return UART_NUM_1;
    case Port::Port2:
      return UART_NUM_2;
    default:
      return UART_NUM_MAX;
  }
}