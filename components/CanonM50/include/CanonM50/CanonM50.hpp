#ifndef __CANONM50_H__
#define __CANONM50_H__

#include <CanonM50/ConnectivityState.hpp>
#include <CanonM50/Scan.hpp>

#include <NimBLEDevice.h>

#include <string>

using nvs_handle_t = uint32_t;

class CanonM50 {
 public:
  CanonM50(const std::string& name);
  ~CanonM50();

  void init();

  bool pair(const NimBLEAddress& address);

  bool connect();

  bool disconnect();

  bool isConnected() const;

  bool trigger();

  Scan* getScan();

 private:

  NimBLEClient* m_client;
  ConnectivityState m_connectionState;
  NimBLEAddress m_lastDeviceAddress;
  NimBLERemoteService* m_remoteService;
  NimBLERemoteCharacteristic* m_pairingCharacteristic;
  NimBLERemoteCharacteristic* m_shutterCharacteristic;

  std::string m_name;
  Scan m_scan;
  nvs_handle_t m_nvs;
};

#endif // __CANONM50_H__