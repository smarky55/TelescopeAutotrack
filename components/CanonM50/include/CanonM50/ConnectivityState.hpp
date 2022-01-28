#ifndef __CONNECTIVITYSTATE_H__
#define __CONNECTIVITYSTATE_H__

#include <NimBLEClient.h>

class ConnectivityState : public NimBLEClientCallbacks {
 public:
  bool isConnected() const;

 protected:
  void onConnect(NimBLEClient*) override;

  void onDisconnect(NimBLEClient*) override;

  bool onConfirmPIN(uint32_t pin) override;

  uint32_t onPassKeyRequest() override;

  void onAuthenticationComplete(ble_gap_conn_desc* desc) override;

 protected:
  bool m_connected = false;
};

#endif  // __CONNECTIVITYSTATE_H__