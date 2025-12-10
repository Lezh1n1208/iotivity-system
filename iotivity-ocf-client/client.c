#include "oc_api.h"
#include "oc_endpoint.h"
#include "oc_rep.h"
#include "port/oc_clock.h"
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int quit = 0;
static oc_endpoint_t *server_ep = NULL;

static void signal_handler(int sig) {
  (void)sig;
  quit = 1;
}

static void temp_response(oc_client_response_t *data) {
  if (data && data->code == OC_STATUS_OK && data->payload) {
    double temp = 0.0;
    if (oc_rep_get_double(data->payload, "temperature", &temp)) {
      printf("🌡️  Temperature: %.1f°C\n", temp);
    }
  }
  fflush(stdout);
}

static void humid_response(oc_client_response_t *data) {
  if (data && data->code == OC_STATUS_OK && data->payload) {
    double humid = 0.0;
    if (oc_rep_get_double(data->payload, "humidity", &humid)) {
      printf("💧 Humidity: %.1f%%\n", humid);
    }
  }
  fflush(stdout);
}

static oc_endpoint_t *create_endpoint(const char *host, uint16_t port) {
  oc_endpoint_t *ep = oc_new_endpoint();
  if (!ep)
    return NULL;

  memset(ep, 0, sizeof(oc_endpoint_t));
  ep->flags = IPV4;
  ep->addr.ipv4.port = port;

  struct in_addr addr;
  if (inet_pton(AF_INET, host, &addr) == 1) {
    memcpy(ep->addr.ipv4.address, &addr, 4);
    return ep;
  }

  oc_free_endpoint(ep);
  return NULL;
}

static oc_discovery_flags_t
discovery_cb(const char *anchor, const char *uri, oc_string_array_t types,
             oc_interface_mask_t iface, const oc_endpoint_t *endpoint,
             oc_resource_properties_t bm, void *user_data) {
  (void)anchor;
  (void)types;
  (void)iface;
  (void)bm;
  (void)user_data;
  (void)endpoint;

  if (uri && strstr(uri, "/temperature") && !server_ep) {
    printf("✅ Discovered: %s\n", uri);
    server_ep = create_endpoint("172.20.0.10", 5683);
    fflush(stdout);
    return OC_STOP_DISCOVERY;
  }
  return OC_CONTINUE_DISCOVERY;
}

static int app_init(void) {
  int ret = oc_init_platform("IoTClient", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.wk.d", "OCF Client", "ocf.2.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  return ret;
}

static void signal_event_loop(void) {}

int main(void) {
  setbuf(stdout, NULL);
  printf("\n🔌 OCF Client\n\n");

  signal(SIGINT, signal_handler);

  static const oc_handler_t handler = {.init = app_init,
                                       .signal_event_loop = signal_event_loop};

  if (oc_main_init(&handler) < 0) {
    printf("❌ Init failed\n");
    return -1;
  }

  printf("🔍 Discovering server...\n");
  fflush(stdout);

  oc_do_ip_discovery("oic.r.temperature", discovery_cb, NULL);

  for (int i = 0; i < 100 && !server_ep; i++) {
    oc_main_poll_v1();
    usleep(100000);
  }

  if (!server_ep) {
    printf("❌ Server not found\n");
    oc_main_shutdown();
    return -1;
  }

  printf("✅ Connected to server\n\n");
  fflush(stdout);

  time_t last_req = 0;

  while (!quit) {
    oc_main_poll_v1();

    time_t now = time(NULL);
    if (now - last_req >= 10) {
      oc_do_get("/temperature", server_ep, NULL, temp_response, LOW_QOS, NULL);
      oc_do_get("/humidity", server_ep, NULL, humid_response, LOW_QOS, NULL);
      last_req = now;
    }
    usleep(10000);
  }

  if (server_ep)
    oc_free_endpoint(server_ep);
  oc_main_shutdown();
  return 0;
}