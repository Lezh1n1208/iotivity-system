#include "oc_api.h"
#include "oc_endpoint.h"
#include "oc_rep.h"
#include "port/oc_clock.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int quit = 0;
static oc_endpoint_t *server_ep = NULL;
static int discovery_done = 0;

static void signal_handler(int sig) {
  (void)sig;
  quit = 1;
}

// ✅ FIX: Sử dụng oc_rep_get_double() API đúng
static void temp_response(oc_client_response_t *data) {
  printf("\n[Client] ===== RESPONSE /temperature =====\n");
  fflush(stdout);

  if (!data) {
    printf("❌ NULL response!\n");
    fflush(stdout);
    return;
  }

  printf("[Client] Code: %d\n", data->code);
  fflush(stdout);

  // ✅ Sử dụng API đúng để đọc double value
  double temp_value = 0.0;
  if (oc_rep_get_double(data->payload, "temperature", &temp_value)) {
    printf("🌡️  Temperature: %.1f°C\n", temp_value);
  } else {
    printf("⚠️  Could not parse temperature from payload\n");
  }
  fflush(stdout);
}

static void humid_response(oc_client_response_t *data) {
  printf("\n[Client] ===== RESPONSE /humidity =====\n");
  fflush(stdout);

  if (!data) {
    printf("❌ NULL response!\n");
    fflush(stdout);
    return;
  }

  printf("[Client] Code: %d\n", data->code);
  fflush(stdout);

  // ✅ Sử dụng API đúng để đọc double value
  double humid_value = 0.0;
  if (oc_rep_get_double(data->payload, "humidity", &humid_value)) {
    printf("💧 Humidity: %.1f%%\n", humid_value);
  } else {
    printf("⚠️  Could not parse humidity from payload\n");
  }
  fflush(stdout);
}

// Tạo endpoint trỏ đến Docker server
static void create_server_endpoint(void) {
  char *server_ip_str = getenv("SERVER_IP");
  if (!server_ip_str) {
    server_ip_str = "172.20.0.10";
  }

  printf("[Client] Creating endpoint for %s:5683...\n", server_ip_str);
  fflush(stdout);

  server_ep = oc_new_endpoint();
  if (!server_ep) {
    printf("❌ Failed to create endpoint\n");
    fflush(stdout);
    return;
  }

  server_ep->flags = IPV4;
  server_ep->device = 0;
  server_ep->addr.ipv4.port = 5683;

  int a, b, c, d;
  if (sscanf(server_ip_str, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
    server_ep->addr.ipv4.address[0] = a;
    server_ep->addr.ipv4.address[1] = b;
    server_ep->addr.ipv4.address[2] = c;
    server_ep->addr.ipv4.address[3] = d;
  }

  printf("✅ Endpoint created:\n");
  printf("   - IP: %s\n", server_ip_str);
  printf("   - Port: 5683\n");
  printf("   - Device: 0\n");
  fflush(stdout);

  discovery_done = 1;
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

  if (strstr(uri, "/temperature") || strstr(uri, "/humidity")) {
    printf("✅ Discovered resource: %s\n", uri);
    fflush(stdout);
  }

  return OC_CONTINUE_DISCOVERY;
}

static int app_init(void) {
  int ret = oc_init_platform("IoTClient", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.wk.d", "OCF Client", "ocf.2.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  printf("✅ Client initialized\n");
  fflush(stdout);
  return ret;
}

static void signal_event_loop(void) {}

int main(void) {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  signal(SIGINT, signal_handler);

  static const oc_handler_t handler = {.init = app_init,
                                       .signal_event_loop = signal_event_loop};

  if (oc_main_init(&handler) < 0) {
    printf("❌ Failed to initialize OCF\n");
    return -1;
  }

  printf("🔍 Starting discovery...\n");
  fflush(stdout);

  oc_do_ip_discovery("oic.r.temperature", discovery_cb, NULL);

  sleep(2);
  create_server_endpoint();

  time_t last_request = 0;
  int request_count = 0;

  while (!quit) {
    oc_main_poll_v1();

    time_t now = time(NULL);

    if (server_ep && (now - last_request >= 10)) {
      request_count++;
      printf("\n📤 Request #%d (%ld)\n", request_count, now);
      fflush(stdout);

      printf("[Client] Sending GET /temperature...\n");
      fflush(stdout);

      if (oc_do_get("/temperature", server_ep, NULL, temp_response, LOW_QOS,
                    NULL)) {
        printf("✅ GET /temperature sent\n");
      } else {
        printf("❌ Failed to send GET /temperature\n");
      }
      fflush(stdout);

      usleep(500000); // 500ms delay

      printf("[Client] Sending GET /humidity...\n");
      fflush(stdout);

      if (oc_do_get("/humidity", server_ep, NULL, humid_response, LOW_QOS,
                    NULL)) {
        printf("✅ GET /humidity sent\n");
      } else {
        printf("❌ Failed to send GET /humidity\n");
      }
      fflush(stdout);

      last_request = now;
    }

    usleep(10000); // 10ms sleep to reduce CPU
  }

  if (server_ep)
    oc_free_endpoint(server_ep);
  oc_main_shutdown();
  return 0;
}