#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <vector>

namespace UART {

enum class Port {
  Port0,  // This port is usually the UART-USB bridge
  Port1,
  Port2
};

enum class Parity { Disable, Even, Odd };

enum class StopBits { Bits_1, Bits_1_5, Bits_2 };

enum class FlowControl { Disable, RTS, CTS, Both };

enum class ClockSource { APB, RefTick };

constexpr int PIN_NO_CHANGE = -1;

struct Config {
  int txPin = PIN_NO_CHANGE;
  int rxPin = PIN_NO_CHANGE;
  int rtsPin = PIN_NO_CHANGE;
  int ctsPin = PIN_NO_CHANGE;
  int baud;
  Parity parity = Parity::Disable;
  StopBits stopBits = StopBits::Bits_1;
  FlowControl flowControl = FlowControl::Disable;
  int flowCtrlRxThresh = 0;
  ClockSource clockSource = ClockSource::APB;
  int txBufferSize = 2048;
  int rxBufferSize = 2048;
};

class UARTDriver {
 public:
  UARTDriver(Port port, const Config& config);
  virtual ~UARTDriver();

  void setBaudRate(int baud);

  void setParity(Parity parity);

  void setStopBits(StopBits stopBits);

  void setFlowControl(FlowControl ctrl, int rxThresh);

  void setPins(int txPin = PIN_NO_CHANGE,
               int rxPin = PIN_NO_CHANGE,
               int rtsPin = PIN_NO_CHANGE,
               int ctsPin = PIN_NO_CHANGE);

  void install(int txBufferSize, int rxBufferSize);

  void remove();

  size_t available();

  size_t sendByte(uint8_t data);
  size_t sendBytes(const std::vector<uint8_t>& data);

  uint8_t receiveByte();
  void receiveBytes(std::vector<uint8_t>& data);

 private:
  const Port m_port;
  constexpr int getImplPort() const;

  void* m_eventQueue;
};

}  // namespace UART

#endif  // __UART_H__