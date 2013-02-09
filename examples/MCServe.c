#include "../lib/CServe.h"

#define DEFAULT_BIG_ENDIAN

char* MC_ReadString(Socket sock){
    unsigned short length = ReadShort(sock);
    int i = 0;
    char* str = calloc(1,length+1);
    while(i < length){
        ReadByte(sock); // move the socket pointer
                str[i] = ReadByte(sock);
        ++i;
    }
    return str;
}

void MC_WriteString(Socket sock, char* message){
    unsigned short length = strlen(message);
    WriteShort(sock, length);
    int i = 0;
    while(i < length){
        WriteByte(sock, 0x00);
        WriteByte(sock, message[i]);
        ++i;
    }
}

void HandleClient(int clientID, Session session){
    int ClientConnected = 1;
    
    while(ClientConnected){
        unsigned char packetType = ReadByte(session.sock);
        switch(packetType){
            case 0x02: // Minecraft Modern Handshake
                printf("Handshake request");
                unsigned char protocolVersion = ReadByte(session.sock);
                char* username = MC_ReadString(session.sock);
                char* serverHost = MC_ReadString(session.sock);
                unsigned int port = ReadInt(session.sock);
                
                WriteByte(session.sock, 0xFF); // kick packet
                char kickMessage[1024];
                sprintf(kickMessage, "Version %d is a great MC Version, %s. Apparently, I'm located at %s:%d. Oh, btw, %s..you're kicked", protocolVersion, username, serverHost, port, username);
                MC_WriteString(session.sock, kickMessage);
                close(session.sock);
                ClientConnected = 0;
                break;
        }
    }
}

int main(int argc, char** argv){
    int socket = CreateServerSocket(25565);
    if(socket == -1){
        printf("Uh-oh!\n");
        exit(0);
    }
    ServerMainLoop(socket, 20, HandleClient);
}