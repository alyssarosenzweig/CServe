#include "../lib/CServe.h"

void HandleRequest(int clientID, __CSERVE_Session session){
    close(session.sock);
}

int main(int argc, char** argv){
    int socket = CreateServerSocket(8089);
    if(socket == -1){
        printf("An error occurred");
    }
    ServerMainLoop(socket, 20, HandleRequest);
}