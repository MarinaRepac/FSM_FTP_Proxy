#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include "./kernelTypes.h"

#define ADDRESS "127.0.0.1"
#define SERVER_PORT 21
#define PROXY_PORT 3000
//#define TIMER1_ID 0

#define BUFFER_SIZE 512

// Automate types
const uint8 PROXY_FSM = 0x00;

// Message boxes
const uint8 PROXY_MBX_ID = 0x00;

// Messages
const uint16 MSG_ConnectingToChrome				= 0x0000;
const uint16 MSG_ConnectingToServer				= 0x0001;
const uint16 MSG_UserCheck						= 0x0002;

#endif //_COMMON_H_