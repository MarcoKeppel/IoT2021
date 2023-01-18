// TODO: enums or macros for states?

#ifndef __STATES_H__
#define __STATES_H__

// enum slave_states{  };
// typedef slave_states slave_states_t;

// Device just powered on, needs to load config from flash
#define SS_INIT         0
// Device needs to find the master
#define SS_MASTER_REQ   1
// Device needs to send sensors list to master
#define SS_SENS_ADV     2
// Device is running and periodically sends updates to master
#define SS_SENS_UPD     3

#define MS_INIT     0
#define MS_RUNNING  1

#endif
