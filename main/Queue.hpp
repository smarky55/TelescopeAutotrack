#ifndef _INCLUDE_QUEUE_H
#define _INCLUDE_QUEUE_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <optional>

template <typename itemT>
class Queue {
 public:
  Queue(UBaseType_t length);
  ~Queue();

  bool send(itemT item, TickType_t ticksToWait);

  std::optional<itemT> receive(TickType_t ticksToWait);

  UBaseType_t messagesWaiting() const;

  UBaseType_t spacesAvailable() const;

 private:
  QueueHandle_t m_handle;
};

template <typename itemT>
Queue<itemT>::Queue(UBaseType_t length) {
  m_handle = xQueueCreate(length, sizeof(itemT));
}

template <typename itemT>
Queue<itemT>::~Queue() {
  vQueueDelete(m_handle);
}

template <typename itemT>
bool Queue<itemT>::send(itemT item, TickType_t ticksToWait) {
  return xQueueSend(m_handle, static_cast<void*>(&item), ticksToWait);
}

template <typename itemT>
std::optional<itemT> Queue<itemT>::receive(TickType_t ticksToWait) {
  itemT item;
  if (xQueueReceive(m_handle, static_cast<void*>(&item), ticksToWait)) {
    return item;
  }
  return std::nullopt;
}

template <typename itemT>
UBaseType_t Queue<itemT>::messagesWaiting() const {
  return uxQueueMessagesWaiting(m_handle);
}

template <typename itemT>
UBaseType_t Queue<itemT>::spacesAvailable() const {
  return uxQueueSpacesAvailable(m_handle);
}

#endif