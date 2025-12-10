// filepath:
// /home/lezh1n/Workspace/Project/IoT/source-code/iotivity-ocf-server/server.c
#include "oc_api.h"
#include "port/oc_clock.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static int quit = 0;
static double g_temperature = 25.0;
static double g_humidity = 60.0;

static void signal_handler(int sig) {
  (void)sig;
  quit = 1;
}

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
  return ret;
}

static void register_resources(void) {
  // Resource /temperature
  oc_resource_t *res_temp = oc_new_resource(NULL, "/temperature", 1, 0);
  oc_resource_bind_resource_type(res_temp, "oic.r.temperature");
  oc_resource_bind_resource_interface(res_temp, OC_IF_R);
  oc_resource_set_default_interface(res_temp, OC_IF_R);
  oc_resource_set_discoverable(res_temp, true);
  oc_resource_set_request_handler(res_temp, OC_GET, get_temperature, NULL);
  oc_add_resource(res_temp);

  // Resource /humidity
  oc_resource_t *res_humid = oc_new_resource(NULL, "/humidity", 1, 0);
  oc_resource_bind_resource_type(res_humid, "oic.r.humidity");
  oc_resource_bind_resource_interface(res_humid, OC_IF_R);
  oc_resource_set_default_interface(res_humid, OC_IF_R);
  oc_resource_set_discoverable(res_humid, true);
  oc_resource_set_request_handler(res_humid, OC_GET, get_humidity, NULL);
  oc_add_resource(res_humid);

  printf("✅ Server initialized with 2 resources:\n");
  printf("   - /temperature (oic.r.temperature)\n");
  printf("   - /humidity (oic.r.humidity)\n");
  fflush(stdout);
}

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
    printf("❌ oc_main_init failed\n");
    return -1;
  }

  printf("🚀 OCF Server running on port 5683\n");
  printf("   Press Ctrl+C to exit\n\n");
  fflush(stdout);

  time_t last_update = 0;

  while (!quit) {
    oc_main_poll_v1();

    time_t now = time(NULL);
    if (now - last_update >= 5) {
      g_temperature += (rand() % 10 - 5) * 0.1;
      g_humidity += (rand() % 10 - 5) * 0.1;

      if (g_temperature < 15)
        g_temperature = 15;
      if (g_temperature > 35)
        g_temperature = 35;
      if (g_humidity < 30)
        g_humidity = 30;
      if (g_humidity > 90)
        g_humidity = 90;

      printf("[Server] Data updated: %.1f°C, %.1f%%\n", g_temperature,
             g_humidity);
      fflush(stdout);
      last_update = now;
    }

    usleep(10000);
  }

  oc_main_shutdown();
  return 0;
}