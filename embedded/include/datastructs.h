#include <Arduino.h>

/*
    Master
*/

#define MAX_SLAVES_N 8
#define MAX_SLAVE_SENSORS_N 8


enum sensor_type { analog, digital, i2c };
enum sensor_val_type { v_int, v_uint, v_real, v_bool };

typedef sensor_type sensor_type_t;
typedef sensor_val_type sensor_val_type_t;


typedef struct {

    const char* name;                 // Human readable name
    sensor_type_t type;
    sensor_val_type_t val_type;
    uint8_t update_rate;
    //unsigned char val[4];       // float* f = (float*) &val;
    int32_t val;        // Should be more generic than int, but for now this will work ok
    uint8_t pin;

} sensor_t;

typedef struct {

    uint32_t addr;
    char* name;
    sensor_t sensors[MAX_SLAVE_SENSORS_N];
    uint32_t keepalive_period;

} slave_t;
