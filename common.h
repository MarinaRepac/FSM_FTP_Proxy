#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include "./kernelTypes.h"

#define ADDRESS "127.0.0.1"
#define SERVER_PORT 21
#define PROXY_PORT 3000
#define DATA_PORT 20
#define BUFFER_SIZE 1000

// message buffer size
#define BUFFER_SIZE 2000

// Automate types
const uint8 PROXY_FSM = 0x00;

// Message boxes
const uint8 PROXY_MBX_ID = 0x00;

// Messages
const uint16 MSG_ConnectingToChrome				= 0x0000;
const uint16 MSG_ConnectingToServer				= 0x0001;
const uint16 MSG_UserCheck						= 0x0002;
const uint16 MSG_PasswordCheck					= 0x0003;
const uint16 MSG_LoggedIn						= 0x0004;
const uint16 MSG_ConnectingPort1024				= 0x0005;
const uint16 MSG_Download						= 0x0006;
const uint16 MSG_Upload							= 0x0007;
const uint16 MSG_Disconnecting					= 0x0008;

#endif //_COMMON_H_