// common file for definitions, e.g. parameter lengths, possible node IDs, frequency settings, EEPROM locations etc.


#ifndef COMMON_H
#define COMMON_H

//ID ranges
#define MAX_NODE_ID 31
#define MAX_GROUP_ID 212

#define MIN_NODE_ID 0
#define MIN_GROUP_ID 1

//factory FREQUENCY of JeeLink nodes (radio modules are marked with yellow dot)
#define FREQUENCY RF12_868MHZ

#define SERIAL_FREQUENCY 57600

//maximum length for RF12 message format
#define MAX_MESSAGE_LENGTH 20

#define MAX_NODES 30

//count of routing algorithm cycles
#define ROUTING_CYCLES 50

#define TIMEOUT 3000

//EEPROM values locations
#define NODE_ID_LOCATION    0
#define GROUP_ID_LOCATION   1
#define PARENT_ID_LOCATION  2

//Serial output level
// 0 - debug
// 1 - output
// 2 - error
#define OUTPUT_LEVEL 0


#endif //COMMON_H
