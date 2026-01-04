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
static int temp_unavailable = 0;
static int humid_unavailable = 0;
static bool discovery_done = false;

static void signal_handler(int sig) {
  (void)sig;
  quit = 1;
}

// ==================== OBSERVE Handlers ====================
static void temp_observe_handler(oc_client_response_t *data) {
  if (!data) {
    printf("❌ Temperature: No response\n");
    temp_unavailable = 1;
    fflush(stdout);
    return;
  }

  if (data->code == OC_STATUS_SERVICE_UNAVAILABLE) {
    printf("⚠️  Temperature: SENSOR OFFLINE\n");
    temp_unavailable = 1;
    fflush(stdout);
    return;
  }

  if (data->code == OC_STATUS_OK && data->payload) {
    double temp = 0.0;
    char *source = NULL;
    size_t source_len = 0;
    
    if (oc_rep_get_double(data->payload, "temperature", &temp)) {
      printf("🌡️  [OBSERVE] Temperature: %.1f°C", temp);
      if (oc_rep_get_string(data->payload, "source", &source, &source_len)) {
        printf(" [%s]", source);
      }
      printf("\n");
      temp_unavailable = 0;
    }
  }
  fflush(stdout);
}

static void humid_observe_handler(oc_client_response_t *data) {
  if (!data) {
    printf("❌ Humidity: No response\n");
    humid_unavailable = 1;
    fflush(stdout);
    return;
  }

  if (data->code == OC_STATUS_SERVICE_UNAVAILABLE) {
    printf("⚠️  Humidity: SENSOR OFFLINE\n");
    humid_unavailable = 1;
    fflush(stdout);
    return;
  }

  if (data->code == OC_STATUS_OK && data->payload) {
    double humid = 0.0;
    char *source = NULL;
    size_t source_len = 0;
    
    if (oc_rep_get_double(data->payload, "humidity", &humid)) {
      printf("💧 [OBSERVE] Humidity: %.1f%%", humid);
      if (oc_rep_get_string(data->payload, "source", &source, &source_len)) {
        printf(" [%s]", source);
      }
      printf("\n");
      humid_unavailable = 0;
    }
  }
  fflush(stdout);
}

// ==================== Discovery Callback ====================
static oc_discovery_flags_t
discovery_cb(const char *anchor, const char *uri, oc_string_array_t types,
             oc_interface_mask_t iface, const oc_endpoint_t *endpoint,
             oc_resource_properties_t bm, void *user_data) {
  (void)anchor;
  (void)types;
  (void)iface;
  (void)bm;
  (void)user_data;

  // ✅ Discovery thật sự - không hardcode IP
  if (uri && strstr(uri, "/temperature") && !server_ep && endpoint) {
    printf("✅ Discovered OCF Server:\n");
    printf("   URI: %s\n", uri);
    
    // Extract IP from discovered endpoint
    char ip[INET_ADDRSTRLEN];
    if (endpoint->flags & IPV4) {
      inet_ntop(AF_INET, endpoint->addr.ipv4.address, ip, sizeof(ip));
      printf("   IP: %s:%d\n", ip, endpoint->addr.ipv4.port);
      
      // Clone endpoint từ discovery result
      server_ep = oc_new_endpoint();
      memcpy(server_ep, endpoint, sizeof(oc_endpoint_t));
      discovery_done = true;
      
      fflush(stdout);
      return OC_STOP_DISCOVERY;
    }
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
  printf("\n🔌 OCF Client (OCF-Compliant)\n");
  printf("================================\n\n");

  signal(SIGINT, signal_handler);

  static const oc_handler_t handler = {.init = app_init,
                                       .signal_event_loop = signal_event_loop};

  if (oc_main_init(&handler) < 0) {
    printf("❌ Init failed\n");
    return -1;
  }

  // ==================== Phase 1: Discovery ====================
  printf("🔍 Phase 1: Discovering OCF servers...\n");
  printf("   (multicast to 224.0.1.187:5683)\n");
  fflush(stdout);

  oc_do_ip_discovery("oic.r.temperature", discovery_cb, NULL);

  // Đợi discovery hoàn thành (timeout 30s)
  time_t start = time(NULL);
  while (!discovery_done && (time(NULL) - start) < 30 && !quit) {
    oc_main_poll_v1();
    usleep(100000);
  }

  if (!server_ep) {
    printf("\n❌ No OCF Server found after 30s!\n");
    printf("💡 Troubleshooting:\n");
    printf("   1. Check if ocf-server container is running\n");
    printf("   2. Verify network connectivity between containers\n");
    printf("   3. Check firewall: sudo ufw allow 5683/udp\n");
    printf("   4. Ensure multicast is enabled in Docker network\n");
    oc_main_shutdown();
    return -1;
  }

  printf("✅ Discovery completed!\n\n");

  // ==================== Phase 2: Observe Resources ====================
  printf("🔍 Phase 2: Setting up OBSERVE on resources...\n");
  fflush(stdout);

  // Setup OBSERVE cho /temperature
  if (!oc_do_observe("/temperature", server_ep, NULL, 
                     temp_observe_handler, LOW_QOS, NULL)) {
    printf("❌ Failed to observe /temperature\n");
  } else {
    printf("✅ Observing /temperature\n");
  }

  // Setup OBSERVE cho /humidity
  if (!oc_do_observe("/humidity", server_ep, NULL, 
                     humid_observe_handler, LOW_QOS, NULL)) {
    printf("❌ Failed to observe /humidity\n");
  } else {
    printf("✅ Observing /humidity\n");
  }

  printf("\n📡 Waiting for notifications from server...\n");
  printf("   (Press Ctrl+C to exit)\n\n");
  fflush(stdout);

  // ==================== Phase 3: Event Loop ====================
  while (!quit) {
    oc_main_poll_v1();
    usleep(10000);  // Server sẽ push updates tự động
  }

  if (server_ep)
    oc_free_endpoint(server_ep);
  oc_main_shutdown();
  return 0;
}