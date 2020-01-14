#include "proxy.h"
#include <WinSock.h>

#include <iostream>
using namespace std;

// proxy address
sockaddr_in proxyAddress, serverAddress, chromeAddress;

// sockets
SOCKET serverSocket, chromeSocket, clientSocket;

char buffer[200];

ProxyAutomate::ProxyAutomate() : FiniteStateMachine( PROXY_FSM, PROXY_MBX_ID, 10, 10, 10) {
}

ProxyAutomate::~ProxyAutomate() {
}


uint8 ProxyAutomate::GetAutomate() {
	return PROXY_FSM;
}

uint8 ProxyAutomate::GetMbxId() {
	return PROXY_MBX_ID;
}

MessageInterface *ProxyAutomate::GetMessageInterface(uint32 id) {
  if(id == 0) 
	  return &StandardMsgCoding;
  throw TErrorObject( __LINE__, __FILE__, 0x01010400);
}

void ProxyAutomate::SetDefaultHeader(uint8 infoCoding) {
	SetMsgInfoCoding(infoCoding);
	SetMessageFromData();
}

void ProxyAutomate::SetDefaultFSMData() {

}

void ProxyAutomate::NoFreeInstances() {
	printf("[%d] AutoExample::NoFreeInstances()\n", GetObjectId());
}

void ProxyAutomate::Initialize() {
	SetState(IDLE);
	SetDefaultFSMData();

	InitEventProc(IDLE, MSG_ConnectingToChrome, (PROC_FUN_PTR)&ProxyAutomate::connectingToChrome);
	InitEventProc(CONNECTING, MSG_ConnectingToServer, (PROC_FUN_PTR)&ProxyAutomate::connectingToFTP);
	InitEventProc(USER_CHECK, MSG_UserCheck, (PROC_FUN_PTR)&ProxyAutomate::user_check);
}

/* Initial system message */
void ProxyAutomate::Start() {
	PrepareNewMessage(0x00, MSG_ConnectingToChrome);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
}

void ProxyAutomate::connectingToChrome() {
	printf("-----------------------------------------------------------\n"); 
	printf("CONNECTING TO GOOGLE CHROME\n");
	printf("-----------------------------------------------------------\n"); 

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
    }

    // Initialize proxyAddress structure used by bind function
	memset((char*)&proxyAddress, 0, sizeof(proxyAddress));
    proxyAddress.sin_family = AF_INET; 							
	proxyAddress.sin_addr.S_un.S_addr = inet_addr(ADDRESS);			
	proxyAddress.sin_port = htons(PROXY_PORT);					

    // Create a proxy socket for Chrome
    serverSocket = socket(AF_INET,		// IPv4 address famly
						  SOCK_STREAM,   // stream socket
						  0);			// TCP

	// Check if socket creation succeeded
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
    }

	// Bind proxy address structure (type, port number and local address) to socket
    int iResult = bind(serverSocket,(SOCKADDR *)&proxyAddress, sizeof(proxyAddress));

	// Check if socket is succesfully binded to server datas
    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
    }

	listen(serverSocket, SOMAXCONN);
	printf("Waiting for incoming connections to proxy...\n");

	chromeSocket = socket(AF_INET,		// IPv4 address famly
				   SOCK_STREAM,			// stream socket
				   0);					// TCP

	chromeSocket = accept(serverSocket, (struct sockaddr *)&proxyAddress, NULL);
	printf("Chrome connection request arrived\n");
	printf("accepted chromeSocket = %d\n", chromeSocket);

	PrepareNewMessage(0x00, MSG_ConnectingToServer);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(CONNECTING);
}

void ProxyAutomate::connectingToFTP() {
	printf("-----------------------------------------------------------\n");
	printf("CONNECTING TO FTP SERVER\n");
	printf("-----------------------------------------------------------\n");

	serverAddress.sin_family = AF_INET; 							
	serverAddress.sin_addr.S_un.S_addr = inet_addr(ADDRESS);			
	serverAddress.sin_port = htons(SERVER_PORT);	

	clientSocket = socket(AF_INET,		 // IPv4 address famly
						  SOCK_STREAM,   // stream socket
						  0);			 // TCP

	if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		printf("Socket connect failed with error: %d\n", WSAGetLastError());
	}

	printf("Proxy connected to server...\n");

	memset(buffer, 0, 200);
	int bytes = recv(clientSocket, buffer, 200, 0);
	if (bytes > 0)
	{
		printf("%s\n", buffer);
	}

	printf("Sending from server to Chrome...\n");

	if (send(chromeSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	printf("Send to Chrome...\n");

	PrepareNewMessage(0x00, MSG_UserCheck);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(USER_CHECK);
}

void ProxyAutomate::user_check() {
	printf("-----------------------------------------------------------\n");
	printf("USER CHECK\n");
	printf("-----------------------------------------------------------\n");

	printf("Recv from Chrome...\n");

	memset(buffer, 0, 200);
	if (recv(chromeSocket, buffer, 200, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("%s\n", buffer);

	/*PrepareNewMessage(0x00, MSG_UserCheck);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(USER_CHECK);*/
}