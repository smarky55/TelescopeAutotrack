#include "webserver.hpp"

#include <esp_log.h>

#include <cmath>

static const char* TAG = "Webserver";

static std::function<esp_err_t(httpd_req_t* request)> s_commandCallback;

static esp_err_t root_get_handler(httpd_req_t* request) {
  extern const unsigned char indexHtmlStart[] asm("_binary_index_html_start");
  extern const unsigned char indexHtmlEnd[] asm("_binary_index_html_end");
  const size_t indexHtmlSize = indexHtmlEnd - indexHtmlStart;
  return httpd_resp_send(request, (const char*)indexHtmlStart, indexHtmlSize);
}

static const httpd_uri_t root = {.uri = "/",
                                 .method = HTTP_GET,
                                 .handler = root_get_handler,
                                 .user_ctx = nullptr};

static esp_err_t scope_js_get_handler(httpd_req_t* request) {
  extern const unsigned char scopeJsStart[] asm("_binary_scope_js_start");
  extern const unsigned char scopeJsEnd[] asm("_binary_scope_js_end");
  const size_t scopeJsSize = scopeJsEnd - scopeJsStart;
  httpd_resp_set_type(request, "application/javascript");
  return httpd_resp_send(request, (const char*)scopeJsStart, scopeJsSize);
}

static const httpd_uri_t scopeJs = {.uri = "/scope.js",
                                    .method = HTTP_GET,
                                    .handler = scope_js_get_handler,
                                    .user_ctx = nullptr};

static esp_err_t core_css_get_handler(httpd_req_t* request) {
  extern const unsigned char coreCssStart[] asm("_binary_core_css_start");
  extern const unsigned char coreCssEnd[] asm("_binary_core_css_end");
  const size_t coreCssSize = coreCssEnd - coreCssStart;
  httpd_resp_set_type(request, "text/css");
  return httpd_resp_send(request, (const char*)coreCssStart, coreCssSize);
}

static const httpd_uri_t coreCss = {.uri = "/core.css",
                                    .method = HTTP_GET,
                                    .handler = core_css_get_handler,
                                    .user_ctx = nullptr};

static esp_err_t command_post_handler(httpd_req_t* request) {
  return s_commandCallback(request);
  // char buf[100];
  // int ret, remaining = request->content_len;

  // while (remaining > 0) {
  //   /* Read the data for the request */
  //   if ((ret = httpd_req_recv(
  //            request, buf, std::min((unsigned)remaining, sizeof(buf)))) <= 0) {
  //     if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
  //       /* Retry receiving if timeout occurred */
  //       continue;
  //     }
  //     return ESP_FAIL;
  //   }

  //   /* Send back the same data */
  //   httpd_resp_send_chunk(request, buf, ret);
  //   remaining -= ret;

  //   /* Log data received */
  //   ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
  //   ESP_LOGI(TAG, "%.*s", ret, buf);
  //   ESP_LOGI(TAG, "====================================");
  // }

  // // End response
  // httpd_resp_send_chunk(request, NULL, 0);
  // return ESP_OK;
}

static const httpd_uri_t commandPost = {.uri = "/command",
                                        .method = HTTP_POST,
                                        .handler = command_post_handler,
                                        .user_ctx = nullptr};

httpd_handle_t webserverStart() {
  httpd_handle_t server = nullptr;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.core_id = 0;

  ESP_LOGI(TAG, "Starting server on port: %d", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    ESP_LOGI(TAG, "Registering URI Handlers");

    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &scopeJs);
    httpd_register_uri_handler(server, &coreCss);
    httpd_register_uri_handler(server, &commandPost);

    return server;
  }

  ESP_LOGE(TAG, "Failed to start webserver!");
  return nullptr;
}

void webserverStop(httpd_handle_t server) {
  httpd_stop(server);
}

void webserverConnectHandler(void* arg,
                    esp_event_base_t eventBase,
                    int32_t eventId,
                    void* enventData) {
  httpd_handle_t* server = static_cast<httpd_handle_t*>(arg);
  if (*server == nullptr) {
    *server = webserverStart();
  }
}

void webserverDisconnectHandler(void* arg,
                       esp_event_base_t eventBase,
                       int32_t eventId,
                       void* enventData) {
  httpd_handle_t* server = static_cast<httpd_handle_t*>(arg);
  if (*server) {
    webserverStop(*server);
    *server = nullptr;
  }
}

void webserverSetCommandCallback(std::function<esp_err_t(httpd_req_t*)> callback) {
  s_commandCallback = callback;
}
