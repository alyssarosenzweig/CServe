#include "../lib/CServe.h"

void HandleRequest(int clientID, __CSERVE_Session session){
    char** fullRequest = ReadLineUntilDelim(session.sock, '\r');
    char** request = SplitRequest(fullRequest[0],' ');
    printf("Split\n");
    
    char* requestType = request[0];
    char* fileName = request[1];
    char* httpVersion = request[2];
    
    printf("initiatied\n");
    
   // httpVersion = httpVersion - 1; // sneaky way to cut last character
    
    char buffer[512];
    sprintf(buffer, "HTTP/1.0 200 OK\n\nHi! This a is an example! You requested %s!", fileName);
    write(session.sock, buffer, strlen(buffer));
    
    close(session.sock);
}

int main(int argc, char** argv){
    int socket = CreateServerSocket(8888);
    if(socket == -1){
        printf("Uh-oh! An error occurred");
        exit(0);
    }
    ServerMainLoop(socket, 20, HandleRequest);
    return 0;
}