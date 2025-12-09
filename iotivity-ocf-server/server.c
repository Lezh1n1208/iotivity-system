#include "oc_api.h"
#include "port/oc_clock.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int quit = 0;

// Simulated sensor data
static double g_temperature = 25.5;
static double g_humidity = 60.0;

static void signal_handler(int sig) {
  (void)sig;
  quit = 1;
}

// Handler cho GET /temperature
static void get_temperature(oc_request_t *req, oc_interface_mask_t iface,
                            void *data) {
  (void)iface;
  (void)data;

  printf("[Server] GET /temperature → %.1f°C\n", g_temperature);
  fflush(stdout);  

  oc_rep_start_root_object();
  oc_rep_set_double(root, temperature, g_temperature);
  oc_rep_end_root_object();

  oc_send_response(req, OC_STATUS_OK);
}

// Handler cho GET /humidity
static void get_humidity(oc_request_t *req, oc_interface_mask_t iface,
                         void *data) {
  (void)iface;
  (void)data;

  printf("[Server] GET /humidity → %.1f%%\n", g_humidity);
  fflush(stdout);  

  oc_rep_start_root_object();
  oc_rep_set_double(root, humidity, g_humidity);
  oc_rep_end_root_object();

  oc_send_response(req, OC_STATUS_OK);
}

static int app_init(void) {
  int ret = oc_init_platform("IoTServer", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.sensor", "Temperature Sensor",
                       "ocf.2.0.0", "ocf.res.1.0.0", NULL, NULL);

  // Tạo resource /temperature
  oc_resource_t *res_temp = oc_new_resource(NULL, "/temperature", 1, 0);
  oc_resource_bind_resource_type(res_temp, "oic.r.temperature");
  oc_resource_bind_resource_interface(res_temp, OC_IF_R);
  oc_resource_set_discoverable(res_temp, true);
  oc_resource_set_request_handler(res_temp, OC_GET, get_temperature, NULL);
  oc_add_resource(res_temp);

  // Tạo resource /humidity
  oc_resource_t *res_humid = oc_new_resource(NULL, "/humidity", 1, 0);
  oc_resource_bind_resource_type(res_humid, "oic.r.humidity");
  oc_resource_bind_resource_interface(res_humid, OC_IF_R);
  oc_resource_set_discoverable(res_humid, true);
  oc_resource_set_request_handler(res_humid, OC_GET, get_humidity, NULL);
  oc_add_resource(res_humid);

  printf("✅ Server initialized with 2 resources:\n");
  printf("   - /temperature (oic.r.temperature)\n");
  printf("   - /humidity (oic.r.humidity)\n");
  fflush(stdout);  

  return ret;
}

static void register_resources(void) {}
static void signal_event_loop(void) {}

int main(void) {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  signal(SIGINT, signal_handler);
  srand(time(NULL));

  static oc_handler_t handler = {.init = app_init,
                                 .signal_event_loop = signal_event_loop,
                                 .register_resources = register_resources};

  if (oc_main_init(&handler) < 0) {
    return -1;
  }

  printf("🚀 OCF Server running on port 5683\n");
  printf("   Press Ctrl+C to exit\n\n");
  fflush(stdout);  

  while (!quit) {
    oc_main_poll_v1();

    // Simulate sensor update mỗi 5 giây
    static time_t last_update = 0;
    time_t now = time(NULL);
    if (now - last_update >= 5) {
      g_temperature += (rand() % 10 - 5) * 0.1;
      g_humidity += (rand() % 10 - 5) * 0.1;
      printf("[Server] Data updated: %.1f°C, %.1f%%\n", g_temperature, g_humidity);
      fflush(stdout);  
      last_update = now;
    }
  }

  oc_main_shutdown();
  return 0;
}