#include "proxy.h"
#include <WinSock.h>

#include <fstream>
#include <iostream>
using namespace std;

// address structures
sockaddr_in proxyAddress, serverAddress, proxyAddress1024, serverAddress1024, proxyAddressRETR, serverAddressRETR;

// sockets
SOCKET serverSocket, browserSocket, clientSocket, serverSocket1024, browserSocket1024, clientSocket1024, serverSocketRETR, browserSocketRETR, clientSocketRETR;

// message buffer
char buffer[BUFFER_SIZE];

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
	SetState(CONNECTING);
	SetDefaultFSMData();

	InitEventProc(CONNECTING, MSG_ConnectingToChrome, (PROC_FUN_PTR)&ProxyAutomate::connectingToChrome);
	InitEventProc(CONNECTING, MSG_ConnectingToServer, (PROC_FUN_PTR)&ProxyAutomate::connectingToFTP);
	InitEventProc(AUTHENTICATION, MSG_UserCheck, (PROC_FUN_PTR)&ProxyAutomate::user_check);
	InitEventProc(AUTHENTICATION, MSG_PasswordCheck, (PROC_FUN_PTR)&ProxyAutomate::pass_check);
	InitEventProc(LOGGED_IN, MSG_LoggedIn, (PROC_FUN_PTR)&ProxyAutomate::logged_in);
	InitEventProc(CONNECTING, MSG_ConnectingPort1024, (PROC_FUN_PTR)&ProxyAutomate::connecting_port_1024);
	InitEventProc(RETR, MSG_Download, (PROC_FUN_PTR)&ProxyAutomate::retr);
	InitEventProc(STOR, MSG_Upload, (PROC_FUN_PTR)&ProxyAutomate::stor);
	InitEventProc(QUIT, MSG_Disconnecting, (PROC_FUN_PTR)&ProxyAutomate::disconnect);
}

/* Initial system message */
void ProxyAutomate::Start() {
	PrepareNewMessage(0x00, MSG_ConnectingToChrome);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
}

void ProxyAutomate::connectingToChrome() {
	printf("\n-----------------------------------------------------------\n"); 
	printf("CONNECTING - GOOGLE CHROME\n");
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
	proxyAddress.sin_addr.S_un.S_addr = inet_addr(ADDRESS);			
	proxyAddress.sin_port = htons(21);					

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

void ProxyAutomate::connectingToFTP() {
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
	//serverAddress.sin_addr.S_un.S_addr = inet_addr(ADDRESS);	
	serverAddress.sin_addr.S_un.S_addr = inet_addr("10.81.35.53");	
	//serverAddress.sin_addr.S_un.S_addr = inet_addr("192.168.1.11");	
	//serverAddress.sin_addr.S_un.S_addr = inet_addr("10.81.35.62");	
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
	memset(buffer, 0, BUFFER_SIZE);
	int bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);
	if (bytes > 0)
	{
		printf("--- FTP_SERVER: %s", buffer);
	}

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	PrepareNewMessage(0x00, MSG_UserCheck);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(AUTHENTICATION);
}

void ProxyAutomate::user_check() {
	printf("\n-----------------------------------------------------------\n");
	printf("USER CHECK\n");
	printf("-----------------------------------------------------------\n\n");

	// USER ****
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	PrepareNewMessage(0x00, MSG_PasswordCheck);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(AUTHENTICATION);
}

void ProxyAutomate::pass_check() {
	printf("\n-----------------------------------------------------------\n");
	printf("PASSWORD CHECK\n");
	printf("-----------------------------------------------------------\n\n");

	// 331 Password required for ****
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// PASS ****
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	PrepareNewMessage(0x00, MSG_LoggedIn);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(LOGGED_IN);
}

void ProxyAutomate::logged_in() {
	printf("\n-----------------------------------------------------------\n");
	printf("LOGGED IN\n");
	printf("-----------------------------------------------------------\n\n");

	// 230 User successfully logged in.
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// SYST (return system type)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 215 UNIX emulated by Quick 'n Easy FTP Server.
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// FEAT (get the feature list implemented by the server)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 211-Extensions supported: SIZE, MDTM, XCRC
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s\n", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// PWD (print working directory; returns the current directory of the host)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 257 "/" is current directory.
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// TYPE I (sets the transfer mode - binary) 
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 200 Type set to BINARY
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// PASV (enter passive mode)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 227 Entering Passive Mode (127,0,0,1,4,0)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// CWD / (change working directory)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	PrepareNewMessage(0x00, MSG_ConnectingPort1024);
	SetMsgToAutomate(PROXY_FSM);
	SetMsgObjectNumberTo(GetObjectId());
	SendMessage(PROXY_MBX_ID);
	SetState(CONNECTING);
}

void ProxyAutomate::connecting_port_1024() {
	printf("\n-----------------------------------------------------------\n");
	printf("CONNECTING - SWITCHING TO PORT 1024\n");
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
	proxyAddress1024.sin_addr.S_un.S_addr = inet_addr(ADDRESS);			
	proxyAddress1024.sin_port = htons(1024);					

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

	// aktivni soket            pasivni soket
	browserSocket1024 = accept(serverSocket1024, (struct sockaddr *)&proxyAddress1024, NULL);

	printf("--- Browser connection request arrived...\n");

	serverAddress1024.sin_family = AF_INET; 					
	//serverAddress1024.sin_addr.S_un.S_addr = inet_addr(ADDRESS);
	serverAddress1024.sin_addr.S_un.S_addr = inet_addr("10.81.35.53");
	//serverAddress1024.sin_addr.S_un.S_addr = inet_addr("192.168.1.11");	
	//serverAddress1024.sin_addr.S_un.S_addr = inet_addr("10.81.35.62");
	serverAddress1024.sin_port = htons(1024);	

	// Create a proxy socket for server
	clientSocket1024 = socket(AF_INET,			// IPv4 address famly
						      SOCK_STREAM,		// stream socket
						      0);				// TCP

	if (connect(clientSocket1024, (struct sockaddr *)&serverAddress1024, sizeof(serverAddress1024)) < 0) {
		printf("Socket connect failed with error: %d\n", WSAGetLastError());
	}

	printf("--- Proxy connected to server...\n\n");
	
	// 250 "/" is current directory. 
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// LIST (returns information of a file or directory if specified, else information of the current working directory is returned)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// receiving list of files from server
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket1024, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket1024, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 150 Opening ASCII mode data connection for directory list.
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 226 Transfer complete
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	PrepareNewMessage(0x00, MSG_Download);
		SetMsgToAutomate(PROXY_FSM);
		SetMsgObjectNumberTo(GetObjectId());
		SendMessage(PROXY_MBX_ID);
		SetState(RETR);

	

	/*int x;
	if ((x = connect(clientSocket1024, (struct sockaddr *)&serverAddress1024, sizeof(serverAddress1024))) < 0) {
		printf("Socket connect failed with error: %d\n", WSAGetLastError());
	}
	cout << "CONNECT " << x << endl;
	printf("--- Proxy connected to server...\n\n");*/



	/*char command_word[5];

	for(int i = 0; i < 4; i++) {
		command_word[i] = buffer[i];
	}
	command_word[4] = 0;

	printf("--- COMMAND WORD: %s", command_word);*/

	/*if (buffer[0] == 'R') {
		PrepareNewMessage(0x00, MSG_Download);
		SetMsgToAutomate(PROXY_FSM);
		SetMsgObjectNumberTo(GetObjectId());
		SendMessage(PROXY_MBX_ID);
		SetState(RETR);
	} else if (buffer[0] == 'S') {
		PrepareNewMessage(0x00, MSG_Upload);
		SetMsgToAutomate(PROXY_FSM);
		SetMsgObjectNumberTo(GetObjectId());
		SendMessage(PROXY_MBX_ID);
		SetState(STOR);
	} else if (buffer[0] == 'Q') {
		PrepareNewMessage(0x00, MSG_Disconnecting);
		SetMsgToAutomate(PROXY_FSM);
		SetMsgObjectNumberTo(GetObjectId());
		SendMessage(PROXY_MBX_ID);
		SetState(QUIT);
	}*/

}

void ProxyAutomate::retr() {
	printf("\n-----------------------------------------------------------\n");
	printf("RETR - DOWNLOADING\n");
	printf("-----------------------------------------------------------\n\n");

	// PASV
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 227 Entering Passive Mode (192,168,1,11,4,0)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}



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
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 213 15 (server return size of the file for download)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// MDTM /file_name (return the last-modified time of a specified file)
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 213 20200116211436.000
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// RETR /file_name
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(browserSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- BROWSER: %s", buffer);

	if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	// 150 Opening BINARY mode data connection for file transfer.
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}

	






	

	// receiving file

	// od servera na 1024
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocketRETR, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocketRETR, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}




	memset(buffer, 0, BUFFER_SIZE);
	if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
		printf("Recv failed with error: %d\n", WSAGetLastError());
	}

	printf("--- FTP_SERVER: %s", buffer);

	if (send(browserSocket, buffer, strlen(buffer), 0) < 0) {
		printf("Send message failed with error: %d\n", WSAGetLastError());
	}
}

void ProxyAutomate::stor() {
	printf("\n-----------------------------------------------------------\n");
	printf("STOR - UPLOADING\n");
	printf("-----------------------------------------------------------\n\n");
	
}

void ProxyAutomate::disconnect() {
	printf("\n-----------------------------------------------------------\n");
	printf("QUIT - DISCONNECTING\n");
	printf("-----------------------------------------------------------\n\n");

}