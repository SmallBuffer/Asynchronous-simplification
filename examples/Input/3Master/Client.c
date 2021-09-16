#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>
#include <time.h>

int Init1()
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
int Init2()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    serv_addr.sin_port = htons(2345);  
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return sock;
}
int getId() //Function to get the serial number of the client
{
    return time(0)%1000000; //Simulate the serial number of the client with a timestamp
}
int main()
{
    int sock_to_server = Init1();
    int sock_to_master = Init2();
    int send,id, recv;
    id = getId();
    write(sock_to_server, &id, sizeof(int)); //send to server
    read(sock_to_server, &recv, sizeof(int));//read from server
    assert(recv==2);
    send = 1;
    write(sock_to_master, &send, sizeof(int)); //send to master
    close(sock_to_server);//close socket
    close(sock_to_master);

    return 0;
}

/*
1 => pong
2 => ping
3 => ack
*/
