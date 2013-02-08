#include "../lib/CServe.h"

void HandleRequest(int clientID, __CSERVE_Session session){
    char buffer[256];
    for(;;){
        read(session.sock, buffer, 255);
        if(strncmp(buffer,"quit",4) == 0){
            close(session.sock);
            break;
        } else {
            write(session.sock, buffer, strlen(buffer));
        }
    }
}

int main(int argc, char** argv){
    int socket = CreateServerSocket(8088);
    if(socket == -1){
        printf("An error occurred\n");
        exit(0);
    }
    ServerMainLoop(socket, 20, HandleRequest);
}