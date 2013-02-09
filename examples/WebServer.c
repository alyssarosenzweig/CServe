#include "../lib/CServe.h"

void HandleRequest(int clientID, __CSERVE_Session session){
    char** fullRequest = ReadLineUntilDelim(session.sock, '\r');
    char** request = SplitRequest(fullRequest[0],' ');
    
    char* requestType = request[0];
    char* fileName = request[1];
    char* httpVersion = request[2];
    
    httpVersion = httpVersion - 1; // sneaky way to cut last character
    
    char buffer[512];
    sprintf(buffer, "%s 200 OK\n\nHello! You used a %s request on page %s using %s", httpVersion, requestType, fileName, httpVersion);
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