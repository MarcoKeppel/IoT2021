#ifndef __DATASTRUCTS_H__
#define __DATASTRUCTS_H__

#include <Arduino.h>
#include "config.h"

enum sensor_type
{
    analog,
    digital,
    i2c
};
enum sensor_val_type
{
    v_int,
    v_uint,
    v_real,
    v_bool
};

enum node_role
{
    master,
    slave,
    master_slave
};

typedef sensor_type sensor_type_t;
typedef sensor_val_type sensor_val_type_t;

typedef node_role node_role_t;

typedef struct
{

    const char *name; // Human readable name
    sensor_type_t type;
    sensor_val_type_t val_type;
    uint8_t update_rate;
    // unsigned char val[4];       // float* f = (float*) &val;
    int32_t val; // Should be more generic than int, but for now this will work ok
    uint8_t pin;

} sensor_t;

typedef struct
{

    uint32_t addr;
    char *name;
    sensor_t sensors[M_MAX_SLAVE_SENSORS_N];
    uint8_t n_sensors = 0;
    int32_t keepalive_period = -1;
    int32_t keepalive_counter;
    int32_t kill_countdown = KEEPALIVE_KILL_PERIODS;

} slave_t;

#endif
