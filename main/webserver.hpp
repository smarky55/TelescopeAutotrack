#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <esp_event.h>
#include <esp_http_server.h>

#include <functional>

httpd_handle_t webserverStart();

void webserverStop(httpd_handle_t server);

void webserverConnectHandler(void* arg,
                    esp_event_base_t eventBase,
                    int32_t eventId,
                    void* enventData);

void webserverDisconnectHandler(void* arg,
                       esp_event_base_t eventBase,
                       int32_t eventId,
                       void* enventData);

void webserverSetCommandCallback(std::function<esp_err_t(httpd_req_t*)> callback);

#endif  // __WEBSERVER_H__