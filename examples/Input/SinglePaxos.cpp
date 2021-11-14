#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <queue>
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#define client_num  20
#define server_num  100

std::queue<int> server_buffer[server_num],client_buffer[client_num];
const int round = 100;

struct PROPOSAL
{
	int 	id;
	int	serialNum;
	int	value;
	int	is_agreed;
	int	type;
};
void Send(std::queue<int>* addr,void * x,int len)
{
	addr->push(x);
}
void Recv(std::queue<int>* addr,void* res,int len )
{
	while(addr->empty());	//缓存区为空时读取阻塞
	*res = addr->front();
	addr->pop();
}


void * Server_Thread(void * par)
{


	bool prepared; 
	prepared = false;
	int id,index, nextindex, lastLogIndex, minProposal, firstUnchosenIndex, acceptedProposal[100], acceptedValue[100],maxRound;
	index = 0;
	nextindex = 0;
	maxRound = 0;
	unsigned int N, value, accepted_num;
	unsigned int temp_serialNum;
	int agree_num;
	PROPOSAL proposal_recv, proposal_send;

	id = *((int * )par);
	value = id;

	while(true)
	{
		N = time(0)%10000;
		while (true)
		{   
			agree_num = 0;
			temp_serialNum = 0;
			N = N + 5;
			for (int i = 0; i < client_num;++i) // Iterate over multiple-client
			{
				proposal_send.serialNum = N;
				proposal_send.type = 0; 
				proposal_send.id = id;
				Send(&client_buffer[i], &proposal_send, sizeof(PROPOSAL));
			}
			for (int i = 0; i < client_num;++i) // Iterate over multiple-client
			{
				Recv(&client_buffer[i], &proposal_recv, sizeof(PROPOSAL));
				if(proposal_recv.is_agreed == 1)
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
		proposal_send.type = 1;
		proposal_send.id = id;
		agree_num = 0;
		for (int i = 0; i < client_num;++i) // Iterate over multiple-client
		{
			Send(&client_buffer[i], &proposal_send, sizeof(PROPOSAL));
		}
		for (int i = 0; i < client_num;++i) // Iterate over multiple-client
		{
			Recv(&client_buffer[i], &proposal_recv, sizeof(PROPOSAL));
			if(proposal_recv.is_agreed)
				agree_num = agree_num + 1;
		}
		if( 2 * agree_num > client_num)break;
	}
	assert(value == server_num); 
	return 0;
}

void * Client_Thread(void * par)
{


	unsigned int resN,acceptN,acceptV,N;
	PROPOSAL proposal_recv, proposal_send;
	int id_p,id_q;
	id_q = *((int *)par);
	while(true) //Repeat
	{
		Recv(&client_buffer[id_q],&proposal_recv,sizeof(PROPOSAL));
		id_p = proposal_recv.id;
		if(proposal_recv.type == 0)
		{
			N = proposal_recv.serialNum;
			if(N>resN)
			{
				resN = N;
				proposal_send.serialNum = acceptN;
				proposal_send.value = acceptV;
			}
			else
			{
				proposal_send.is_agreed = 0;
			}
		}
		else if(proposal_recv.type == 1)
		{
			if(proposal_recv.serialNum>resN)
			{
				resN = proposal_recv.serialNum;
				acceptN = proposal_recv.serialNum;
				acceptV = proposal_recv.value;
				proposal_send.is_agreed = 1;
			}
			else
			{
				proposal_send.is_agreed = 0;
			}
		}
		Send(&server_buffer[id_p], &proposal_send, sizeof(PROPOSAL)); //send to server
	}
	return 0;
}

int main()
{
	//start thread
	for(int i=0;i<server_num;++i){
		pthread_t id;
		int ret = 0;

		ret = pthread_create(&id, NULL,Server_Thread,NULL);
		pthread_join(id,NULL);
	}

	for( int i=0 ; i < client_num ; ++i)
	{

		pthread_t id;
		int ret = 0;

		ret = pthread_create(&id, NULL,Client_Thread, &i);
		pthread_join(id,NULL);

	}

}
