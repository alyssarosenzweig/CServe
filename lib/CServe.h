#ifndef __CSERVE_MAIN_HEADER_INCLUDED__
#define __CSERVE_MAIN_HEADER_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>


typedef int bool;
#define TRUE 1
#define FALSE 0

typedef int Socket;

typedef struct{
    Socket sock; // socket connection
    bool isConnected; // is it connected?
}__CSERVE_Session;

char* ReadLine(int sock);
int CreateServerSocket(int port);
int ServerMainLoop(int tsocket, int maxClients, void (*externalHandler)(int,__CSERVE_Session));
char** SplitRequest(char* str, char delimiter);
char** ReadLineUntilDelim(Socket sock, char delim);


#endif