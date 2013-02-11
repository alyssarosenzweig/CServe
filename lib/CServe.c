#define MAX_SERVER_SOCKETS 2

#include "CServe.h"

pthread_t* __CSERVE_threads = NULL;
int __CSERVE_clilens[MAX_SERVER_SOCKETS];


long* __CSERVE_task_data;

__CSERVE_Session* __CSERVE_msgsock = NULL;

int __CSERVE_highestSession = 0;
void (*__CSERVE_clientHandlers[MAX_SERVER_SOCKETS])(int,__CSERVE_Session);

struct sockaddr_in __CSERVE_serv_addrs[MAX_SERVER_SOCKETS], __CSERVE_cli_addrs[MAX_SERVER_SOCKETS];

int* __CSERVE_socketArrNum = NULL;


int numberOfServerSocks = 0;

// SMTP Instant Code
#ifdef ENABLE_INSTANTSMTP
InstantSMTP_Settings* __CSERVE_InstantSMTP_Settings = NULL;

void __CSERVE_InstantSMTP_HandleRequest(int clientID, __CSERVE_Session session){
    write(session.sock, __CSERVE_InstantSMTP_Settings->banner, strlen(__CSERVE_InstantSMTP_Settings->banner));
    
    bool hasHandshaked = 0;
    bool sentRecipents = 0;
    bool sentSender = 0;
    bool sentData = 0;
    
    int genMode = __CSERVE_INSTANTSMTP_COMMAND_MODE;
    
    char* mailFrom;
    
    char** recipents = calloc(1, __CSERVE_INSTANTSMTP_MAX_RECIPENTS);
    int recipentNum = 0;
    
    char commandbuffer[512];
    for(;;){
        char* txtrequest = ReadLine(session.sock);
        if(genMode == __CSERVE_INSTANTSMTP_COMMAND_MODE){
            int numberOfInputs = 0;
            char** request = SplitRequestSafe(txtrequest,' ', &numberOfInputs);
            if(numberOfInputs > 0){
                char* command = request[0];
                
                
                
                if(strequ(command, "HELO")){
                    sprintf(commandbuffer, "250 Hello\n");
                    write(session.sock, commandbuffer, strlen(commandbuffer));
                    hasHandshaked = 1;
                } else if(strequ(command, "EHLO")){
                    sprintf(commandbuffer, "250 Hello\n");
                    write(session.sock, commandbuffer, strlen(commandbuffer));
                    hasHandshaked = 1;
                } else if(strequ(command, "RCPT") && hasHandshaked){
                    if(recipentNum < __CSERVE_INSTANTSMTP_MAX_RECIPENTS && numberOfInputs > 1){
                        recipents[recipentNum] = calloc(1, strlen(request[1]));
                        memcpy(recipents[recipentNum], request[1], strlen(request[1]));
                        ++recipentNum;
                    }
                    sprintf(commandbuffer, "250 Ok\n");
                    write(session.sock, commandbuffer, strlen(commandbuffer));
                    sentRecipents = 1;
                    
                } else if(strequ(command, "MAIL") && hasHandshaked){
                    if(numberOfInputs > 1){
                        mailFrom = calloc(1, strlen(request[1]));
                        memcpy(mailFrom, request[1], strlen(request[1]));
                        sprintf(commandbuffer, "250 Ok\n");
                        write(session.sock, commandbuffer, strlen(commandbuffer));
                        sentSender = 1;
                    }
                } else if(strequ(command, "DATA") && hasHandshaked && sentRecipents && sentSender){
                    genMode = __CSERVE_INSTANTSMTP_DATA_MODE;
                    sprintf(commandbuffer, "354 Ok");
                    write(session.sock, commandbuffer, strlen(commandbuffer));
                } else if(strequ(command, "QUIT")){
                    sprintf(commandbuffer, "221 Bye\n");
                    write(session.sock, commandbuffer, strlen(commandbuffer));
                    close(session.sock);
                    break;
                }
            }
        }else if(genMode == __CSERVE_INSTANTSMTP_DATA_MODE){
            if(txtrequest[0] == '.'){
                // end data
                genMode = __CSERVE_INSTANTSMTP_COMMAND_MODE;
                sentData = 1;
                sprintf(commandbuffer, "250 Recieved message safely\n");
                write(session.sock, commandbuffer, strlen(commandbuffer));
            } else {
                printf("Data: %s", txtrequest);
            }
            
        }
    }

}

int InstantSMTP(InstantSMTP_Settings* settings, int port, int maxClients){
    __CSERVE_InstantSMTP_Settings = settings;
    __CSERVE_ServerSocket* __CSERVE_InstantSMTP_sock = CreateServerSocket(port);
    if(__CSERVE_InstantSMTP_sock == NULL){
        printf("Instant SMTP Internal Error: aborting...\n");
        return -1;
    } else {
        ServerMainLoop(*__CSERVE_InstantSMTP_sock, maxClients, __CSERVE_InstantSMTP_HandleRequest);
    }
    return -1;
}
#endif

// Pure-ASCII-only functions..

char* ReadLine(int sock){
    char* buf = calloc(1, 256);
    char tchar;
    int i = 0;
    int currMax = 256;
    int hardMax = 2048;
    while(read(sock,&tchar,1)){
        buf[i++] = tchar;
        if(tchar == '\n')
            break;
        if(i > currMax && currMax < hardMax){
            buf = realloc(buf, currMax*2);
            currMax *= 2;
        } else if(hardMax < currMax){
            break;
        }
    }
    return buf;
}

char** ReadLineUntilDelim(Socket sock, char delim){
    int maxLines = 32;
    char** lines = calloc(1,maxLines);
    int i = 0;
    for(;;){
        char* ln = ReadLine(sock);
        if(i == maxLines){
            maxLines *= 2;
            lines = realloc(lines, maxLines);
        }
        if(ln[0] == delim)
            break;
        lines[i] = ln;
        ++i;
    }
    return lines;
}

char** SplitRequestSafe(char* str, char delimiter, int* len){
    char* delim = calloc(1,1);
    memcpy(delim, &delimiter,1);
    char* pch = strtok(str, delim);
    char** returnVal = (char**)calloc(sizeof(char*), 32);
    int i = 0;
    int maxParts = 32;
    
    while(pch != NULL){
        returnVal[i] = calloc(1, strlen(pch)+1);
        memcpy(returnVal[i],pch,strlen(pch));
        pch = strtok(NULL, delim);
        ++i;
        if(i == maxParts){
            maxParts *= 2;
            returnVal = realloc(returnVal, maxParts);
        }
    }
    if(len != NULL)
        *len = i;
    free(delim);
    return returnVal;
}


void RecursiveFreeStringArr(char** str){
    int i = 0;
    while(str[i] != 0x00){
        free(str[i]);
        ++i;
    }
    free(str);
}


// binary only functions

// endian code samples based off of code by IBM Computers ( http://www.ibm.com/developerworks/aix/library/au-endianc/index.html?ca=drs- )
#ifdef DEFAULT_BIG_ENDIAN
// use big-endian checking
static inline int is_correctendian(){
	const int i = 1;
	return ( (*(char*)&i) == 0);
}
#else
static inline int is_correctendian(){
    const int i = 0;
    return ((*(char*)&i) != 0);
}
#endif

static inline short reverseShort (short i) {
    unsigned char c1, c2;
	
    if (is_correctendian()) {
        return i;
    } else {
        c1 = i & 255;
        c2 = (i >> 8) & 255;
	}
	return ((int)c1 << 8) | c2;
}

static inline int reverseInt (int i) {
    unsigned char c1, c2, c3, c4;
    
    if (is_correctendian()) {
        return i;
    } else {
        c1 = i & 255;
        c2 = (i >> 8) & 255;
        c3 = (i >> 16) & 255;
        c4 = (i >> 24) & 255;
        
        return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
    }
}


unsigned char ReadByte(Socket sock){
    unsigned char b;
    read(sock, &b, 1);
    return b;
}

 char ReadSByte(Socket sock){
    char b;
    read(sock, &b, 1);
    return b;
}

unsigned short ReadShort(Socket sock){
    unsigned short s;
    read(sock, &s, 2);
    return reverseShort(s);
}

unsigned int ReadInt(Socket sock){
    unsigned int i;
    read(sock, &i, 4);
    return reverseInt(i);
}

void WriteByte(Socket sock, unsigned char b){
    write(sock, &b, 1);
}

void WriteSByte(Socket sock, char b){
    write(sock, &b, 1);
}

void WriteShort(Socket sock, unsigned short s){
    unsigned short rs = reverseShort(s);
    write(sock, &rs, 2);
}

void WriteInt(Socket sock, unsigned int s){
    unsigned int ri = reverseInt(s);
    write(sock, &ri, 4);
}

// general socket functions

void SendToAllClients(char* str, int len){
    int i = 0;
    while(i < __CSERVE_highestSession){
        if(__CSERVE_msgsock[i].isConnected)
            write(__CSERVE_msgsock[i].sock, str, len);
        ++i;
    }
}


void* __CSERVE_InteralThreadFunc(void* client_t){
    int client = *(int*)client_t;
    __CSERVE_clientHandlers[__CSERVE_socketArrNum[client]](client, __CSERVE_msgsock[client]);
    __CSERVE_msgsock[client].isConnected = FALSE;
    pthread_exit(NULL);
}

#ifdef UNPREFIX_INTERNAL
ServerSocket* CreateServerSocket(int port){
#else
__CSERVE_ServerSocket* CreateServerSocket(int port){
#endif
    int serverSockNum = ++numberOfServerSocks;
    int tsocket = socket(AF_INET, SOCK_STREAM, 0);

	if(tsocket < 0){
		printf("ERROR Socketing\n");
        return NULL;
    }
	signal(SIGPIPE, SIG_IGN);
	setsockopt(tsocket, SOL_SOCKET, SO_REUSEADDR, (char*)1, 4);
	setsockopt(tsocket, SOL_SOCKET, SO_LINGER, (char*)0, 4);
	bzero((char*)&__CSERVE_serv_addrs[serverSockNum], sizeof(__CSERVE_serv_addrs[serverSockNum]));
	__CSERVE_serv_addrs[serverSockNum].sin_family = AF_INET;
	__CSERVE_serv_addrs[serverSockNum].sin_addr.s_addr = INADDR_ANY;
	__CSERVE_serv_addrs[serverSockNum].sin_port = htons(port);
	if(bind(tsocket, (struct sockaddr*)&__CSERVE_serv_addrs[serverSockNum], sizeof(__CSERVE_serv_addrs[serverSockNum])) < 0){
		printf("ERROR Binding\n");
		return NULL;
	} else {
		printf("Binded\n");
	}
	fflush(stdout);
	listen(tsocket, 5);
	__CSERVE_clilens[serverSockNum] = sizeof(__CSERVE_cli_addrs[serverSockNum]);
    
#ifdef UNPREFIX_INTERNAL
    ServerSocket* retVal = calloc(1, sizeof(ServerSocket));
#else
    __CSERVE_ServerSocket* retVal = calloc(1, sizeof(__CSERVE_ServerSocket));
#endif
    retVal->sock = tsocket;
    retVal->serverSocketNumber = serverSockNum;
    return retVal;
}

static inline int ComputeSessionNum(){
    int i = 0;
    while(i < __CSERVE_highestSession){
        if(!__CSERVE_msgsock[i].isConnected) break;
        ++i;
    }
    if(i == __CSERVE_highestSession) ++__CSERVE_highestSession;
    return i;
}

int ServerMainLoop(__CSERVE_ServerSocket tsocket, int maxClients, void (*externalHandler)(int,__CSERVE_Session)){
    if(__CSERVE_msgsock == NULL) __CSERVE_msgsock = calloc(sizeof(__CSERVE_Session), maxClients);
    __CSERVE_clientHandlers[tsocket.serverSocketNumber] = externalHandler;
    if(__CSERVE_threads == NULL) __CSERVE_threads = calloc(sizeof(pthread_t), maxClients);
    if(__CSERVE_task_data == NULL) __CSERVE_task_data = calloc(sizeof(long), maxClients);
    if(__CSERVE_socketArrNum == NULL) __CSERVE_socketArrNum = calloc(sizeof(int), maxClients);

    
    for(;;){
        int currSock = accept(tsocket.sock, (struct sockaddr*)&__CSERVE_cli_addrs[tsocket.serverSocketNumber], (socklen_t*)&__CSERVE_clilens[tsocket.serverSocketNumber]);
		if(!currSock){
			printf("ERROR accepting\n");
            // cancel request for safety
        } else {
            int sindex = ComputeSessionNum();
            __CSERVE_task_data[sindex] = sindex;
            __CSERVE_msgsock[sindex].sock = currSock;
            __CSERVE_msgsock[sindex].isConnected = TRUE;
            __CSERVE_socketArrNum[sindex] = tsocket.serverSocketNumber;
            pthread_create(&__CSERVE_threads[sindex], NULL, __CSERVE_InteralThreadFunc, (void*)&__CSERVE_task_data[sindex]);
        }
	}
}
