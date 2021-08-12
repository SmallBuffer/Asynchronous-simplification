#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <assert.h>

using namespace std;
struct Mes{
    string message;
    int c;
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
    string value;
    string client_msg;
    string decision;
    int id;
    Mes mess_recv;
    read(sock, &mess_recv, sizeof(Mes));
    id = mess_recv.c;
    string val;
    val = mess_recv.message;
    if (rand()%2) //用随机数模拟是否确认
    {
        client_msg = "Commit";
    }
    else
    {
        client_msg="Abort";
    };
    write(sock, &client_msg, sizeof(client_msg));
    read(sock, &decision, sizeof(decision));
    if(decision=="Commit"){
        value = val;
    }
    write(sock, "Ack", sizeof("Ack"));
    close(sock); //close socket

    return 0;
}
