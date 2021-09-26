#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <string>
using namespace std;

const int client_num = 20;
const int round = 100;
vector<int> wait_queue;
struct PROPOSAL
{
	int	serialNum;
	int	value;
};

int Init()
{
  int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr)); 
  serv_addr.sin_family = AF_INET;           
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
  serv_addr.sin_port = htons(1234);                   
  bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  return serv_sock;
}
void Connect_Clients(int serv_sock)
{
    while (wait_queue.size()<client_num) {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock =
        accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    wait_queue.push_back(clnt_sock);
  }
}
int main() {
    int serv_sock=Init();

    listen(serv_sock, 20);
    Connect_Clients(serv_sock);

    bool is_leader; //true if current server is leader
    bool prepared; 
    is_leader = true;
    prepared = false;
    int index, nextindex, lastLogIndex, minProposal, firstUnchosenIndex, acceptedProposal[100], acceptedValue[100],maxRound;
    index = 0;
    nextindex = 0;
    maxRound = 0;
    unsigned int N, value, accepted_num;
    unsigned int temp_serialNum;
    int agree_num;
    PROPOSAL proposal_recv, proposal_send;
    while(true)
    {
        if(is_leader == false)
            return 0;
        if (prepared == false)
        {
            index = firstUnchosenIndex;
            nextindex = index + 1;
            maxRound++;
        
            N = time(0)%10000;
            while (true)
            {   
                agree_num = 0;
                temp_serialNum = 0;
                N = N + 5;
                for (int i = 0; i < wait_queue.size(); ++i)
                {
                    write(wait_queue[i], &N, sizeof(int));
                }
                for (int i = 0; i < wait_queue.size(); ++i)
                {
                    read(wait_queue[i], &proposal_recv, sizeof(PROPOSAL));
                    if(proposal_recv.serialNum!=-1&&proposal_recv.value!=-1)
                    {
                        if(proposal_recv.serialNum>temp_serialNum)
                        {
                            value = proposal_recv.value;
                            temp_serialNum = proposal_recv.serialNum;
                        }
                        agree_num++;
                    }    
                }
                if( 2 * agree_num > client_num)break;
            }
            prepared = true;
        }
    }

    while(true)
    {
        N = time(0)%10000;
        while (true)
        {   
            agree_num = 0;
            temp_serialNum = 0;
            N = N + 5;
            for (int i = 0; i < wait_queue.size(); ++i)
            {
                write(wait_queue[i], &N, sizeof(int));
            }
            for (int i = 0; i < wait_queue.size(); ++i)
            {
                read(wait_queue[i], &proposal_recv, sizeof(PROPOSAL));
                if(proposal_recv.serialNum!=-1&&proposal_recv.value!=-1)
                {
                    if(proposal_recv.serialNum>temp_serialNum)
                    {
                        value = proposal_recv.value;
                        temp_serialNum = proposal_recv.serialNum;
                    }
                    agree_num++;
                }    
            }
            if( 2 * agree_num > client_num)break;
        }
        proposal_send.serialNum = N;
        proposal_send.value = value;
        agree_num = 0;
        for (int i = 0; i < wait_queue.size(); ++i)
        {
            write(wait_queue[i], &proposal_send, sizeof(PROPOSAL));
        }
        for (int i = 0; i < wait_queue.size(); ++i)
        {
            bool is_commited;
            read(wait_queue[i], &is_commited, sizeof(bool));
            if(is_commited)
                agree_num = agree_num + 1;
        }
       if( 2 * agree_num > client_num)break;
    }
    
    for(auto clnt_sock:wait_queue)
    close(clnt_sock);
    close(serv_sock);

    return 0;
}
