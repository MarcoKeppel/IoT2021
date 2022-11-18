#include <Arduino.h>

/*
    Master
*/

#define MAX_SLAVES 8
#define MAX_SLAVE_SENSORS_N 8


typedef struct {

    char* name;                 // Human readable name
    sensor_type type;
    sensor_val_type val_type;
    uint8_t update_rate;
    unsigned char val[4];       // float* f = (float*) &val;

} sensor_t;

typedef struct {

    uint32_t addr;
    char* name;
    sensor_t sensors[MAX_SLAVE_SENSORS_N];
    uint32_t keepalive_period;

} slave_t;

enum sensor_type { analog, digital, i2c };
enum sensor_val_type { v_int, v_uint, v_real, v_bool };
