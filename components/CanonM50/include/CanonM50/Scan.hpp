#ifndef __SCAN_H__
#define __SCAN_H__

#include <NimBLEUUID.h>
#include <NimBLEAdvertisedDevice.h>

#include <freertos/semphr.h>

class Scan : public NimBLEAdvertisedDeviceCallbacks {
 public:
  Scan(const NimBLEUUID& serviceUUID);

  std::vector<NimBLEAdvertisedDevice*> devices() const;

  bool start(uint32_t duration);

  bool stop();

  bool isScanning() const;

 protected:
  void onResult(NimBLEAdvertisedDevice* device) override;

  void onScanComplete(NimBLEScanResults);

 private:
  NimBLEUUID m_wantedUUID;
  SemaphoreHandle_t m_mutex;
  bool m_scanning = true;
  std::vector<NimBLEAdvertisedDevice*> m_devices;
};

#endif // __SCAN_H__