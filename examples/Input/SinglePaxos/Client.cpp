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
struct PROPOSAL
{
	unsigned int	serialNum;
	unsigned int	value;
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
    while(server_sock.size()<server_num)
    {
        server_sock.push_back(Init()) ;
    }

    unsigned int resN,acceptN,acceptV;
    PROPOSAL proposal_recv, proposal_send;

    for (int i = 0; i < server_sock.size(); i++)
    {
        int N;
        read(server_sock[i], &N, sizeof(int)); //read from server
        if(N>resN)
        {
            resN = N;
            proposal_send.serialNum = acceptN;
            proposal_send.value = acceptV;
        }
        else
        {
            proposal_send.serialNum = -1 ;
            proposal_send.value = -1;
        }
        write(server_sock[i], &proposal_send, sizeof(PROPOSAL)); //send to server
    }

    for (int i = 0; i < server_sock.size(); i++)
    {
        bool is_commited;
        read(server_sock[i], &proposal_recv, sizeof(PROPOSAL)); //read from server
        if(proposal_recv.serialNum>resN)
        {
            resN = proposal_recv.serialNum;
            acceptN = proposal_recv.serialNum;
            acceptV = proposal_recv.value;
            is_commited = true;
        }
        else
        {
            is_commited = false;
        }
        write(server_sock[i], &is_commited, sizeof(bool)); //send to server
    }
    
    for(auto sock:server_sock)
    close(sock);//close socket

    return 0;
}
