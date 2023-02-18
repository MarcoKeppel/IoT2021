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

    char name[SENSOR_NAME_LEN]; // Human readable name
    sensor_type_t type;
    sensor_val_type_t val_type;
    uint8_t update_rate;
    // unsigned char val[4];       // float* f = (float*) &val;
    union {
        int32_t i;
        uint32_t u;
        float_t f;
        bool b;
    } val;
    uint8_t pin;

    void read() {

        switch (val_type) {

            case v_int:
                val.i = read<int32_t>();
                break;
            
            case v_uint:
                val.u = read<uint32_t>();
                break;

            case v_real:
                val.f = read<float_t>();

            case v_bool:
                val.b = read<bool>();
        }
    }

    template<class T>
    T read() {
        
        switch (type) {
            
            case analog:
                return analogRead(pin);
                break;
            
            case digital:
                return digitalRead(pin);
                break;

            case i2c:
                // TODO: to be implemented
                break;
        }

        return 0;
    }

    template<class T>
    T getValue () {

        switch (val_type) {

            case v_int:
                return val.i;
                break;
            
            case v_uint:
                return val.u;
                break;

            case v_real:
                return val.f;

            case v_bool:
                return val.b;
        }

        return 0;
    }

} sensor_t;

typedef struct
{

    uint32_t addr;
    char name[SLAVE_NAME_LEN];
    sensor_t sensors[M_MAX_SLAVE_SENSORS_N];
    uint8_t n_sensors = 0;
    int32_t keepalive_period = -1;
    int32_t keepalive_counter;
    int32_t kill_countdown = KEEPALIVE_KILL_PERIODS;
    bool is_ready = false;

} slave_t;

#endif
