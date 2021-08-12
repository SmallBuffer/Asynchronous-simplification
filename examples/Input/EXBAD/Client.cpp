#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <assert.h>
char buffer[40];
using namespace std;
int Init()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    serv_addr.sin_port = htons(1234);  
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return sock;
}
int main()
{
    int sock = Init();
    char id[10];
    sprintf(id, "%d", sock);
    write(sock, id, sizeof(id)); //send to server
    char buffer[40];
    read(sock, buffer, sizeof(buffer)-1);//read from server
    assert(strcmp(buffer,"ack")==0);
    close(sock);//close socket

    return 0;
}
