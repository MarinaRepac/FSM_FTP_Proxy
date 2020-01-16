#ifndef _PROXY_H_
#define _PROXY_H_

#include <fsm.h>
#include <fsmsystem.h>
#include "common.h"

#include "../kernel/stdMsgpc16pl16.h"
typedef stdMsg_pc16_pl16 StandardMessage;

class ProxyAutomate : public FiniteStateMachine {
	enum ProxyAutomateStates { CONNECTING, AUTHENTICATION, LOGGED_IN, RETR, STOR, QUIT };

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
	void logged_in();
	void connecting_port_1024();
	void retr();
	void stor();
	void disconnect();

public:
	ProxyAutomate();
	~ProxyAutomate();
	
	void Initialize();
	void Start();
};

#endif /* _PROXY_H_ */