#include "proxy.h"
#include <WinSock.h>
#include <fstream>

sockaddr_in proxyAddress, serverAddress, proxyAddress1024, serverAddress1024, proxyAddressSTOR;

SOCKET serverSocket, browserSocket, clientSocket, serverSocket1024, browserSocket1024, clientSocket1024, 
	   serverSocketRETR, browserSocketRETR, clientSocketRETR;

char buffer[BUFFER_SIZE];

ProxyAutomate::ProxyAutomate() : FiniteStateMachine( PROXY_FSM, PROXY_MBX_ID, 0, 6, 10) {
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
	SetState(CONNECTING);
	SetDefaultFSMData();

	InitEventProc(CONNECTING, MSG_ConnectingToBrowser, (PROC_FUN_PTR)&ProxyAutomate::ConnectingToBrowser);
	InitEventProc(CONNECTING, MSG_ConnectingToServer, (PROC_FUN_PTR)&ProxyAutomate::ConnectingToFTP);
	InitEventProc(AUTHENTICATION, MSG_UserCheck, (PROC_FUN_PTR)&ProxyAutomate::UserCheck);
	InitEventProc(AUTHENTICATION, MSG_PasswordCheck, (PROC_FUN_PTR)&ProxyAutomate::PassCheck);
	InitEventProc(LOGGED_IN, MSG_LoggedIn, (PROC_FUN_PTR)&ProxyAutomate::LoggedIn);
	InitEventProc(SWITCH_PORT, MSG_SwitchPort1024, (PROC_FUN_PTR)&ProxyAutomate::SwitchPort1024);
	InitEventProc(RETR, MSG_Download, (PROC_FUN_PTR)&ProxyAutomate::Retr);
	InitEventProc(QUIT, MSG_Disconnecting, (PROC_FUN_PTR)&ProxyAutomate::Disconnect);
}

/* Initial system message */
void ProxyAutomate::Start() {
	PrepareNewMessage(0x00, MSG_ConnectingToBrowser);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
}

// browser socket (data from browser)
// client socket (data from server)
// send data from browser to server
void ProxyAutomate::SendToServer(SOCKET browser, SOCKET client) {
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browser, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(client, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}
}

// send data from server to browser
void ProxyAutomate::SendToBrowser(SOCKET client, SOCKET browser) {
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(client, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browser, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}
}

void ProxyAutomate::ConnectingToBrowser() {
	printf("\n-----------------------------------------------------------\n"); 
	printf("CONNECTING - BROWSER\n");
	printf("-----------------------------------------------------------\n\n"); 

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
	proxyAddress.sin_addr.S_un.S_addr = inet_addr(PROXY_ADDRESS);			
	proxyAddress.sin_port = htons(PROXY_PORT);					

    // Create a proxy socket for browser
    serverSocket = socket(AF_INET,			// IPv4 address famly
						  SOCK_STREAM,		// stream socket
						  0);				// TCP

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
	printf("--- Waiting for incoming connections to proxy...\n");

	browserSocket = socket(AF_INET,		// IPv4 address famly
				   SOCK_STREAM,			// stream socket
				   0);					// TCP

	browserSocket = accept(serverSocket, (struct sockaddr *)&proxyAddress, NULL);
	printf("--- Browser connection request arrived...\n");

	PrepareNewMessage(0x00, MSG_ConnectingToServer);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(CONNECTING);
}

void ProxyAutomate::ConnectingToFTP() {
	printf("\n-----------------------------------------------------------\n");
	printf("CONNECTING - FTP SERVER\n");
	printf("-----------------------------------------------------------\n\n");

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
    }

	serverAddress.sin_family = AF_INET; 					
	serverAddress.sin_addr.S_un.S_addr = inet_addr(SERVER_ADDRESS);		
	serverAddress.sin_port = htons(SERVER_PORT);	

	// Create a proxy socket for server
	clientSocket = socket(AF_INET,			// IPv4 address famly
						  SOCK_STREAM,		// stream socket
						  0);				// TCP

	if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		printf("Socket connect failed with error: %d\n", WSAGetLastError());
	}

	printf("--- Proxy connected to server...\n");

	// 220 Welcome to Quick 'n Easy FTP Server
	SendToBrowser(clientSocket, browserSocket);

	PrepareNewMessage(0x00, MSG_UserCheck);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(AUTHENTICATION);
}

void ProxyAutomate::UserCheck() {
	printf("\n-----------------------------------------------------------\n");
	printf("USER CHECK\n");
	printf("-----------------------------------------------------------\n\n");

	// USER ****
	SendToServer(browserSocket, clientSocket);

	PrepareNewMessage(0x00, MSG_PasswordCheck);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(AUTHENTICATION);
}

void ProxyAutomate::PassCheck() {
	printf("\n-----------------------------------------------------------\n");
	printf("PASSWORD CHECK\n");
	printf("-----------------------------------------------------------\n\n");

	// 331 Password required for ****
	SendToBrowser(clientSocket, browserSocket);

	// PASS ****
	SendToServer(browserSocket, clientSocket);
		
	PrepareNewMessage(0x00, MSG_LoggedIn);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(LOGGED_IN);
}

void ProxyAutomate::LoggedIn() {
	printf("\n-----------------------------------------------------------\n");
	printf("LOGGED IN\n");
	printf("-----------------------------------------------------------\n\n");

	// 230 User successfully logged in.
	// ...
	// CWD / (change working directory)
	for (int i = 0; i < 6; i++) {
		SendToBrowser(clientSocket,browserSocket );
		SendToServer(browserSocket, clientSocket);
	}

	PrepareNewMessage(0x00, MSG_SwitchPort1024);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(SWITCH_PORT);
}

void ProxyAutomate::SwitchPort1024() {
	printf("\n-----------------------------------------------------------\n");
	printf("SWITCH PORT 1024\n");
	printf("-----------------------------------------------------------\n\n");

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
    }

	// Initialize proxyAddress structure used by bind function
	memset((char*)&proxyAddress1024, 0, sizeof(proxyAddress1024));
    proxyAddress1024.sin_family = AF_INET; 							
	proxyAddress1024.sin_addr.S_un.S_addr = inet_addr(PROXY_ADDRESS);
	proxyAddress1024.sin_port = htons(DATA_PORT);					

    // Create a proxy socket for browser
    serverSocket1024 = socket(AF_INET,			// IPv4 address famly
						      SOCK_STREAM,		// stream socket
						      0);				// TCP

	// Check if socket creation succeeded
    if (serverSocket1024 == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
    }

	// Bind proxy address structure (type, port number and local address) to socket
    int iResult = bind(serverSocket1024,(SOCKADDR *)&proxyAddress1024, sizeof(proxyAddress1024));

	// Check if socket is succesfully binded to server datas
    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket1024);
        WSACleanup();
    }

	listen(serverSocket1024, SOMAXCONN);
	printf("--- Waiting for incoming connections to proxy...\n");

	browserSocket1024 = socket(AF_INET,			// IPv4 address famly
							  SOCK_STREAM,		// stream socket
							  0);				// TCP

	browserSocket1024 = accept(serverSocket1024, (struct sockaddr *)&proxyAddress1024, NULL);

	printf("--- Browser connection request arrived...\n");

	serverAddress1024.sin_family = AF_INET; 					
	serverAddress1024.sin_addr.S_un.S_addr = inet_addr(SERVER_ADDRESS);	
	serverAddress1024.sin_port = htons(DATA_PORT);	

	// Create a proxy socket for server
	clientSocket1024 = socket(AF_INET,			// IPv4 address famly
						      SOCK_STREAM,		// stream socket
						      0);				// TCP

	if (connect(clientSocket1024, (struct sockaddr *)&serverAddress1024, sizeof(serverAddress1024)) < 0) {
		printf("Socket connect failed with error: %d\n", WSAGetLastError());
	}

	printf("--- Proxy connected to server...\n\n");
	
	// 250 "/" is current directory.
	SendToBrowser(clientSocket, browserSocket);

	// LIST (returns information of a file or directory if specified, else information of the current working directory is returned)
	SendToServer(browserSocket, clientSocket);

	// receiving list of files from server

	memset(buffer, 0, BUFFER_SIZE);
	while (recv(clientSocket1024, buffer, BUFFER_SIZE, 0) > 0) {
		printf("--- FTP_SERVER: %s", buffer);

		if (send(browserSocket1024, buffer, strlen(buffer), 0) < 0) {
			printf("Send message failed with error: %d\n", WSAGetLastError());
		}
		memset(buffer, 0, BUFFER_SIZE);
	}

	// 150 Opening ASCII mode data connection for directory list.
	SendToBrowser(clientSocket, browserSocket);

	// 226 Transfer complete
	SendToBrowser(clientSocket, browserSocket);

	PrepareNewMessage(0x00, MSG_Download);
		SetMsgToAutomate(PROXY_FSM);
		SetMsgObjectNumberTo(GetObjectId());
		SendMessage(PROXY_MBX_ID);
		SetState(RETR);
}

void ProxyAutomate::Retr() {
	printf("\n-----------------------------------------------------------\n");
	printf("RETR - DOWNLOADING\n");
	printf("-----------------------------------------------------------\n\n");

	// PASV
	SendToServer(browserSocket, clientSocket);

	// 227 Entering Passive Mode (192,168,1,11,4,0)
	SendToBrowser(clientSocket, browserSocket);

	// opening new sockets for data transfer
	browserSocketRETR = socket(AF_INET,			// IPv4 address famly
							  SOCK_STREAM,		// stream socket
							  0);				// TCP

	browserSocketRETR = accept(serverSocket1024, (struct sockaddr *)&proxyAddress1024, NULL);

	printf("--- Browser connection request arrived...\n");

	// Create a proxy socket for server
	clientSocketRETR = socket(AF_INET,			// IPv4 address famly
						      SOCK_STREAM,		// stream socket
						      0);				// TCP

	if (connect(clientSocketRETR, (struct sockaddr *)&serverAddress1024, sizeof(serverAddress1024)) < 0) {
		printf("Socket connect failed with error: %d\n", WSAGetLastError());
	}

	printf("--- Proxy connected to server...\n\n");

	// SIZE /file_name
	// ...
	// 150 Opening BINARY mode data connection for file transfer.
	for (int i = 0; i < 3; i++) {
		SendToServer(browserSocket, clientSocket);
		SendToBrowser(clientSocket, browserSocket);
	}
	
	// sending file to browser on port 1024

	memset(buffer, 0, BUFFER_SIZE);
	while (recv(clientSocketRETR, buffer, BUFFER_SIZE, 0) > 0) {
		printf("--- FTP_SERVER: %s", buffer);

		if (send(browserSocketRETR, buffer, strlen(buffer), 0) < 0) {
			printf("Send message failed with error: %d\n", WSAGetLastError());
		}
		memset(buffer, 0, BUFFER_SIZE);
	}

	PrepareNewMessage(0x00, MSG_Disconnecting);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(QUIT);
}

void ProxyAutomate::Disconnect() {
	printf("\n-----------------------------------------------------------\n");
	printf("QUIT - DISCONNECTING\n");
	printf("-----------------------------------------------------------\n\n");

	closesocket(browserSocketRETR);

	// 226 Transfer complete
	SendToBrowser(clientSocket, browserSocket);
	printf("--- Connection closed.\n");
}