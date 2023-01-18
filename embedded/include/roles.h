#ifndef __ROLES_H__
#define __ROLES_H__

#include "datastructs.h"
#include "states.h"
#include "config.h"

typedef struct {

    const char* name;
    uint32_t masterAddr;
    sensor_t* sensors;      // Static size array instead? With size defined by MAX_SLAVE_SENSORS_N macro

    uint8_t state;

} slave_data_t;

typedef struct {
    
    char* name;
    slave_t* slaves;        // Static size array instead? With size defined by MAX_SLAVES_N macro

    uint8_t state;

} master_data_t;

#endif
