#include "../lib/CServe.h"

#define COMMAND_MODE 0
#define DATA_MODE 1

#define GREETING "220 localhost ESMTP CServe\n"

#define MAX_RECIPENTS 8

void HandleRequest(int clientID, Session session){
    write(session.sock, GREETING, strlen(GREETING));
    
    bool hasHandshaked = 0;
    bool sentRecipents = 0;
    bool sentSender = 0;
    bool sentData = 0;
    
    int genMode = COMMAND_MODE;
    
    char* mailFrom;
    
    char** recipents = calloc(1, MAX_RECIPENTS);
    int recipentNum = 0;
    
    char commandbuffer[512];
    for(;;){
        char* txtrequest = ReadLine(session.sock);
        if(genMode == COMMAND_MODE){
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
                    if(recipentNum < MAX_RECIPENTS && numberOfInputs > 1){
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
                    genMode = DATA_MODE;
                    sprintf(commandbuffer, "354 End Data with \r\n.\r\n");
                    write(session.sock, commandbuffer, strlen(commandbuffer));
                }
            }
            }else if(genMode == DATA_MODE){
                if(txtrequest[0] == '.'){
                    // end data
                    genMode = COMMAND_MODE;
                    sentData = 1;
                } else {
                    printf("Data: %s", txtrequest);
                }

            }
        }
}

int main(int argc, char** argv){
    ServerSocket* socket = CreateServerSocket(25);
    if(socket == NULL) printf("Error\n");
    else
        ServerMainLoop(*socket, 20, HandleRequest);
    return 0;
}