#include "CServe.h"

pthread_t* __CSERVE_threads;
int __CSERVE_clilen;

long* __CSERVE_task_data;

__CSERVE_Session* __CSERVE_msgsock;

int __CSERVE_highestSession = 0;
void (*__CSERVE_clientHandler)(int,__CSERVE_Session);

    struct sockaddr_in __CSERVE_serv_addr, __CSERVE_cli_addr;


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
