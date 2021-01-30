#ifndef __UARTSTREAM_H__
#define __UARTSTREAM_H__

#include <source/esp32_stream.h>

namespace UART {
class UARTDriver;

class UARTStream : public Stream {
 public:
  UARTStream(UARTDriver* driver);
  ~UARTStream() = default;

  int available() override;
  uint8_t write(const uint8_t data) override;
  int16_t read() override;

 private:
  UARTDriver* m_driver;
  int m_ignoreBytes= 0;
};
}  // namespace UART

#endif  // __UARTSTREAM_H__