#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <assert.h>

using namespace std;
const int round = 20;
struct Mes{
    string message;
    int round;
};
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

    string v[100];
    for (int i = 0; i < round; ++i)
    {
        Mes client_recv,client_send;
        int rid;
        read(sock, &client_recv, sizeof(Mes));
        rid = client_recv.round;
        v[rid] = client_recv.message;
        client_send.message="pong";
        client_send.round = rid;
        write(sock, &client_send, sizeof(Mes));
    }
       

    close(sock);//close socket

    return 0;
}
