#ifndef _PROXY_H_
#define _PROXY_H_

#include <fsm.h>
#include <fsmsystem.h>
#include "common.h"
#include "../kernel/stdMsgpc16pl16.h"

typedef stdMsg_pc16_pl16 StandardMessage;

class ProxyAutomate : public FiniteStateMachine {
	enum ProxyAutomateStates { CONNECTING, AUTHENTICATION, LOGGED_IN, SWITCH_PORT, RETR, QUIT };

	StandardMessage StandardMsgCoding;

	/* FiniteStateMachine abstract functions */
	MessageInterface *GetMessageInterface(uint32 id);
	void	SetDefaultHeader(uint8 infoCoding);
	void	SetDefaultFSMData();
	void	NoFreeInstances();
	uint8	GetMbxId();
	uint8	GetAutomate();

	/* FSM functions */

	void ConnectingToBrowser();
	void ConnectingToFTP();
	void UserCheck();
	void PassCheck();
	void LoggedIn();
	void SwitchPort1024();
	void Retr();
	void Disconnect();

public:
	ProxyAutomate();
	~ProxyAutomate();
	
	void Initialize();
	void Start();
	void SendToServer(SOCKET, SOCKET);
	void SendToBrowser(SOCKET, SOCKET);
};

#endif /* _PROXY_H_ */