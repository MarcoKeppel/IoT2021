#ifndef __CONFIG_H__
#define __CONFIG_H__

#define __FORCE_MASTER__        // Force node to be initialized as master (for debugging purposes)
#define __S_SKIP_SENSOR_ADV__   // Slave skips sensor advertisement state (for debugging purposes)

// TODO: move all configuration params/macros here

#define   MESH_SSID       "UNITN_IOT" 
#define   MESH_PASSWORD   "isthissecureenough?" 
#define   MESH_PORT       2468

#define STATIC_JSON_DOC_SIZE 512

#define SENSORS_UPDATE_PERIOD 1000    // [ms]

#define BROADCAST_PERIOD 2000

#define M_MAX_SLAVES_N 8
#define M_MAX_SLAVE_SENSORS_N 8

const char* configFile = "config.json";     // Config JSON filename (stored in FS)

#endif
