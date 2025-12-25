#include "oc_api.h"
#include "port/oc_clock.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static int quit = 0;
static double g_temperature = 0.0;
static double g_humidity = 0.0;
static int sensor_connected = 0;
static time_t last_sensor_update = 0;

#define STATE_FILE "/tmp/sensor_state.json"

static void signal_handler(int sig) {
  (void)sig;
  quit = 1;
}

static int read_sensor_state() {
  FILE *fp = fopen(STATE_FILE, "r");
  if (!fp) {
    return 0;
  }

  char buffer[512];
  size_t read = fread(buffer, 1, sizeof(buffer) - 1, fp);
  fclose(fp);
  
  if (read == 0) {
    return 0;
  }
  buffer[read] = '\0';

  // Simple JSON parsing (production should use proper library)
  char *temp_str = strstr(buffer, "\"temperature\":");
  char *humid_str = strstr(buffer, "\"humidity\":");
  char *timestamp_str = strstr(buffer, "\"timestamp\":");
  char *connected_str = strstr(buffer, "\"sensor_connected\":");

  if (temp_str && humid_str && timestamp_str && connected_str) {
    double temp, humid;
    long timestamp;
    char connected[10];
    
    if (sscanf(temp_str, "\"temperature\": %lf", &temp) == 1 &&
        sscanf(humid_str, "\"humidity\": %lf", &humid) == 1 &&
        sscanf(timestamp_str, "\"timestamp\": %ld", &timestamp) == 1 &&
        sscanf(connected_str, "\"sensor_connected\": %5s", connected) == 1) {
      
      g_temperature = temp;
      g_humidity = humid;
      last_sensor_update = timestamp;
      sensor_connected = (strncmp(connected, "true", 4) == 0);
      
      return 1;
    }
  }

  return 0;
}

static void get_temperature(oc_request_t *req, oc_interface_mask_t iface,
                            void *data) {
  (void)iface;
  (void)data;

  if (!sensor_connected) {
    printf("⚠️  [Server] GET /temperature → SENSOR OFFLINE\n");
    fflush(stdout);
    oc_send_response(req, OC_STATUS_SERVICE_UNAVAILABLE);
    return;
  }

  printf("✅ [Server] GET /temperature → %.1f°C (real sensor)\n", g_temperature);
  fflush(stdout);

  oc_rep_start_root_object();
  oc_rep_set_double(root, temperature, g_temperature);
  oc_rep_set_text_string(root, source, "ESP8266_DHT11");
  oc_rep_end_root_object();
  oc_send_response(req, OC_STATUS_OK);
}

static void get_humidity(oc_request_t *req, oc_interface_mask_t iface,
                         void *data) {
  (void)iface;
  (void)data;

  if (!sensor_connected) {
    printf("⚠️  [Server] GET /humidity → SENSOR OFFLINE\n");
    fflush(stdout);
    oc_send_response(req, OC_STATUS_SERVICE_UNAVAILABLE);
    return;
  }

  printf("✅ [Server] GET /humidity → %.1f%% (real sensor)\n", g_humidity);
  fflush(stdout);

  oc_rep_start_root_object();
  oc_rep_set_double(root, humidity, g_humidity);
  oc_rep_set_text_string(root, source, "ESP8266_DHT11");
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
  oc_resource_t *res_temp = oc_new_resource(NULL, "/temperature", 1, 0);
  oc_resource_bind_resource_type(res_temp, "oic.r.temperature");
  oc_resource_bind_resource_interface(res_temp, OC_IF_R);
  oc_resource_set_default_interface(res_temp, OC_IF_R);
  oc_resource_set_discoverable(res_temp, true);
  oc_resource_set_request_handler(res_temp, OC_GET, get_temperature, NULL);
  oc_add_resource(res_temp);

  oc_resource_t *res_humid = oc_new_resource(NULL, "/humidity", 1, 0);
  oc_resource_bind_resource_type(res_humid, "oic.r.humidity");
  oc_resource_bind_resource_interface(res_humid, OC_IF_R);
  oc_resource_set_default_interface(res_humid, OC_IF_R);
  oc_resource_set_discoverable(res_humid, true);
  oc_resource_set_request_handler(res_humid, OC_GET, get_humidity, NULL);
  oc_add_resource(res_humid);

  printf("✅ Server initialized: /temperature, /humidity\n");
  fflush(stdout);
}

static void signal_event_loop(void) {}

int main(void) {
  setbuf(stdout, NULL);
  signal(SIGINT, signal_handler);

  static oc_handler_t handler = {.init = app_init,
                                 .signal_event_loop = signal_event_loop,
                                 .register_resources = register_resources};

  if (oc_main_init(&handler) < 0) {
    printf("❌ oc_main_init failed\n");
    return -1;
  }

  printf("🚀 OCF Server running on port 5683\n");
  printf("📡 Waiting for ESP8266 sensor data...\n\n");
  fflush(stdout);

  time_t last_check = 0;

  while (!quit) {
    oc_main_poll_v1();

    time_t now = time(NULL);
    
    // Check sensor state every 2 seconds
    if (now - last_check >= 2) {
      int prev_connected = sensor_connected;
      read_sensor_state();
      
      // Log status changes
      if (!prev_connected && sensor_connected) {
        printf("🟢 ESP8266 Connected! T=%.1f°C, H=%.1f%%\n", 
               g_temperature, g_humidity);
        fflush(stdout);
      } else if (prev_connected && !sensor_connected) {
        printf("🔴 ESP8266 Disconnected (timeout)\n");
        fflush(stdout);
      } else if (sensor_connected) {
        // Quiet update
        time_t age = now - last_sensor_update;
        if (age < 15) {
          printf("📊 Sensor Data: T=%.1f°C, H=%.1f%% (age: %lds)\n", 
                 g_temperature, g_humidity, age);
          fflush(stdout);
        }
      } else {
        printf("⏳ Waiting for ESP8266... (check your WiFi config)\n");
        fflush(stdout);
      }
      
      last_check = now;
    }
    
    usleep(10000);
  }

  oc_main_shutdown();
  return 0;
}