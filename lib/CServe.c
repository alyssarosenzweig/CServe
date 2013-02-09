#include "CServe.h"

pthread_t* __CSERVE_threads;
int __CSERVE_clilen;

long* __CSERVE_task_data;

__CSERVE_Session* __CSERVE_msgsock;

int __CSERVE_highestSession = 0;
void (*__CSERVE_clientHandler)(int,__CSERVE_Session);

struct sockaddr_in __CSERVE_serv_addr, __CSERVE_cli_addr;

// Pure-ASCII-only functions..

char* ReadLine(int sock){
    char* buf = calloc(1, 256);
    char tchar;
    int i = 0;
    int currMax = 256;
    while(read(sock,&tchar,1)){
        buf[i++] = tchar;
        if(tchar == '\n')
            break;
        if(i > currMax){
            buf = realloc(buf, currMax*2);
            currMax *= 2;
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

char** SplitRequest(char* str, char delimiter){
    char* delim = calloc(1,1);
    memcpy(delim, &delimiter,1);
    char* pch = strtok(str, delim);
    char** returnVal = (char**)calloc(sizeof(char*), 32);
    int i = 0;
    int maxParts = 32;
    
    while(pch != NULL){
        printf("%d: %s\n", i, pch);
        returnVal[i] = calloc(1, strlen(pch)+1);
        memcpy(returnVal[i],pch,strlen(pch));
        pch = strtok(NULL, delim);
        ++i;
        if(i == maxParts){
            maxParts *= 2;
            returnVal = realloc(returnVal, maxParts);
        }
    }
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
    __CSERVE_clientHandler(client, __CSERVE_msgsock[client]);
    __CSERVE_msgsock[client].isConnected = FALSE;
    pthread_exit(NULL);
}

int CreateServerSocket(int port){
    int tsocket = socket(AF_INET, SOCK_STREAM, 0);

	if(tsocket < 0){
		printf("ERROR Socketing\n");
		return -1;
	}
	signal(SIGPIPE, SIG_IGN);
	setsockopt(tsocket, SOL_SOCKET, SO_REUSEADDR, (char*)1, 4);
	setsockopt(tsocket, SOL_SOCKET, SO_LINGER, (char*)0, 4);
	bzero((char*)&__CSERVE_serv_addr, sizeof(__CSERVE_serv_addr));
	__CSERVE_serv_addr.sin_family = AF_INET;
	__CSERVE_serv_addr.sin_addr.s_addr = INADDR_ANY;
	__CSERVE_serv_addr.sin_port = htons(port);
	if(bind(tsocket, (struct sockaddr*)&__CSERVE_serv_addr, sizeof(__CSERVE_serv_addr)) < 0){
		printf("ERROR Binding\n");
		return -1;
	} else {
		printf("Binded\n");
	}
	fflush(stdout);
	listen(tsocket, 5);
	__CSERVE_clilen = sizeof(__CSERVE_cli_addr);
    return tsocket;
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

int ServerMainLoop(int tsocket, int maxClients, void (*externalHandler)(int,__CSERVE_Session)){
    __CSERVE_msgsock = calloc(sizeof(__CSERVE_Session), maxClients);
    __CSERVE_clientHandler = externalHandler;
    __CSERVE_threads = calloc(sizeof(pthread_t), maxClients);
    __CSERVE_task_data = calloc(sizeof(long), maxClients);
    
    for(;;){
        int currSock = accept(tsocket, (struct sockaddr*)&__CSERVE_cli_addr, (socklen_t*)&__CSERVE_clilen);
		if(!currSock){
			printf("ERROR accepting\n");
            // cancel request for safety
        } else {
            int sindex = ComputeSessionNum();
            __CSERVE_task_data[sindex] = sindex;
            __CSERVE_msgsock[sindex].sock = currSock;
            __CSERVE_msgsock[sindex].isConnected = TRUE;
            pthread_create(&__CSERVE_threads[sindex], NULL, __CSERVE_InteralThreadFunc, (void*)&__CSERVE_task_data[sindex]);
        }
	}
}
