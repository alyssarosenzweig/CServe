#ifndef __CSERVE_MAIN_HEADER_INCLUDED__
#define __CSERVE_MAIN_HEADER_INCLUDED__

// various includes required by the lib..

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>

// eventually be set in autogen config header
#define UNPREFIX_INTERNAL

// helpful macros

#define strequ(strx,stry) strncmp(strx,stry,strlen(stry)) == 0

// stdbool wasn't working...

typedef int bool;
#define TRUE 1
#define FALSE 0

// complex, interally used structures
typedef int Socket;

typedef struct{
    Socket sock; // socket connection
    bool isConnected; // is it connected?
}__CSERVE_Session;

typedef struct{
    Socket sock;
    int serverSocketNumber;
}__CSERVE_ServerSocket;


#ifdef UNPREFIX_INTERNAL
#define Session __CSERVE_Session
#define ServerSocket __CSERVE_ServerSocket
#endif

// various ASCII functions
char* ReadLine(int sock);
#ifdef UNPREFIX_INTERNAL
ServerSocket* CreateServerSocket(int port);
#else
__CServe_ServerSocket* CreateServerSocket(int port);
#endif
int ServerMainLoop(__CSERVE_ServerSocket tsocket, int maxClients, void (*externalHandler)(int,__CSERVE_Session));
#define SplitRequest(str, delimiter) SplitRequestSafe(str, delimiter, null);
char** SplitRequestSafe(char* str, char delimiterm, int* len);
char** ReadLineUntilDelim(Socket sock, char delim);

// various binary functions

 unsigned char ReadByte(Socket sock);
 char ReadSByte(Socket sock);
 unsigned short ReadShort(Socket sock);
 unsigned int ReadInt(Socket sock);
 void WriteByte(Socket sock, unsigned char b);
 void WriteSByte(Socket sock, char b);
 void WriteShort(Socket sock, unsigned short s);
 void WriteInt(Socket sock, unsigned int s);

#endif