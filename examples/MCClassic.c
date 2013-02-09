#include "../lib/CServe.h"

// C-S packets
#define PLAYER_IDENTIFICATION 0x00

// S-C packets
#define SERVER_IDENTIFICATION 0x00

// Misc.
#define SERVER_NAME "MC-CServe"
#define SERVER_MOTD "Chicken or the egg?"
#define SERVER_VERSION 0x07

#define FLAG_OP 0x64

// CServe constants
#define DEFAULT_BIG_ENDIAN

char* MC_ReadString(Socket sock){
    char* blobData = calloc(1, 64);
    read(sock, blobData, 64);
    //TO-DO: unpad
    return blobData;
}

void MC_WriteString(Socket sock, char* text){
    char* blobData = calloc(1, 64);
    int tlen = strlen(text);
    memcpy(blobData, text, tlen);
    write(sock, blobData, 64);
}

void SendServerIdentification(Socket sock){
    WriteByte(sock, SERVER_IDENTIFICATION);
    WriteByte(sock, SERVER_VERSION);
    MC_WriteString(sock, SERVER_NAME);
    MC_WriteString(sock, SERVER_MOTD);
    WriteByte(sock, FLAG_OP);
}

void HandleClient(int clientID, __CSERVE_Session session){
    int ClientConnected = 1;
    
    while(ClientConnected){
        unsigned char packetID = ReadByte(session.sock);
        switch(packetID){
            case PLAYER_IDENTIFICATION:
                printf("Player identification..");
                unsigned char protocolVersion = ReadByte(session.sock);
                char* username = MC_ReadString(session.sock);
                char* verificationKey = MC_ReadString(session.sock);
                unsigned char unused = ReadByte(session.sock);
                printf("%sv%d (key %s)",username,protocolVersion,verificationKey);
                break;
            default:
                printf("Malformed packet: %d\n", packetID);
                break;
        }
    }
}

int main(int argc, char** argv){
    int socket = CreateServerSocket(1337);
    if(socket == -1){
        printf("Uh-oh!\n");
        exit(0);
    }
    ServerMainLoop(socket, 20, HandleClient);
}