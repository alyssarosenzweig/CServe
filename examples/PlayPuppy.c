#include "../lib/CServe.h"

const char* MoveRequest = "M";
const char* LoginRequest = "L";
const char* SayRequest = "S";

void HandleRequest(int clientID, __CSERVE_Session session){
    char* buffer = calloc(1, 512);
    
    int IsLoggedIn = false;
    
    for(;;){
        char** request = SplitRequest(ReadLine(session.sock),'/');
        
        if(strncmp(request[0],MoveRequest, strlen(MoveRequest)) == 0 && IsLoggedIn){
                sprintf(buffer, "M/%d/%s/%s/\n", clientID, request[1], request[2], request[3]);
                SendToAllClients(buffer, strlen(buffer));
                memset(buffer, 0, 512);
        } else if(strncmp(request[0],LoginRequest, strlen(MoveRequest)) == 0){
                sprintf(buffer, "L/1\n");
                write(session.sock, buffer, strlen(buffer));
                memset(buffer, 0, 512);
                IsLoggedIn = true;
        } else if(strncmp(request[0],SayRequest, strlen(MoveRequest)) == 0 && IsLoggedIn){
                sprintf(buffer, "S/%d/%s/\n", clientID, request[1]);
                SendToAllClients(buffer, strlen(buffer));
                memset(buffer, 0, 512);
        }
    }
}

int main(int argc, char** argv){
    int socket = CreateServerSocket(8089);
    if(socket == -1){
        printf("Uh-oh!\n");
        exit(0);
    }
    ServerMainLoop(socket, 20, HandleRequest);
}