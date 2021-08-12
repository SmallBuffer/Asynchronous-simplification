#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <assert.h>
#include<vector>

using namespace std;
vector<int> server_sock;
const int server_num = 100;
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
    while(server_sock.size()<server_num)
    {
        server_sock.push_back(Init()) ;
    }
    string buffer;
    for (int i = 0; i < server_sock.size();i++)
    {
        read(server_sock[i], &buffer, sizeof(string)); //read from server
        write(server_sock[i], "pong", sizeof("pong")); //send to server
    }
        
    for(auto sock:server_sock)
    close(sock);//close socket

    return 0;
}
