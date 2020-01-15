#ifndef _PROXY_H_
#define _PROXY_H_

#include <fsm.h>
#include <fsmsystem.h>
#include "common.h"

//#define TIMER1_COUNT 5 

#include "../kernel/stdMsgpc16pl16.h"
typedef stdMsg_pc16_pl16 StandardMessage;

class ProxyAutomate : public FiniteStateMachine {
	enum ProxyAutomateStates { IDLE, CONNECTING, AUTHENTICATION, LOG_IN, LOGGED_IN };

	StandardMessage StandardMsgCoding;

	/* FiniteStateMachine abstract functions */
	MessageInterface *GetMessageInterface(uint32 id);
	void	SetDefaultHeader(uint8 infoCoding);
	void	SetDefaultFSMData();
	void	NoFreeInstances();
	uint8	GetMbxId();
	uint8	GetAutomate();

	/* FSM functions */

	void connectingToChrome();
	void connectingToFTP();
	void user_check();
	void pass_check();
	void log_in();
	void logged_in();

public:
	ProxyAutomate();
	~ProxyAutomate();
	
	void Initialize();
	void Start();
};

#endif /* _PROXY_H_ */