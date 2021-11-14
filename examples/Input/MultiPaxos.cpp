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

#define client_num 100
#define server_num 20

std::queue<int> server_buffer[server_num],client_buffer[client_num];

struct PROPOSA 
{
	unsigned int	serialNum;
	unsigned int	value;
};

void Send(std::queue<int>* addr,void * x,int len)
{
	int * tem = (int *)x;
	addr->push(*tem);
	len--;
	while(len--)
	{
		tem++;
		addr->push(*tem);
	}
}
void Recv(std::queue<int>* addr,void * res ,int len)
{
	while(addr->size()<len);	//缓存区不足时读取阻塞
	int * tem = (int *)res;
	*tem=addr->front();
	addr->pop();
	len--;
	while(len--)
	{
		tem++;
		*tem=addr->front();
		addr->pop();
	}
}

void * Client_Thread(void * par)
{

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
	return 0;
}

void* Server_Thread(void * par) 
{
	int is_leader; //true if current server is leader
	int prepared; 
	is_leader = 1;
	prepared = 0;
	int index, nextindex, lastLogIndex, minProposal, firstUnchosenIndex, acceptedProposal[100], acceptedValue[100],maxRound;
	index = 0;
	nextindex = 0;
	maxRound = 0;
	unsigned int N, value, accepted_num;
	unsigned int temp_serialNum;
	int agree_num;
	struct PROPOSAL proposal_recv, proposal_send;
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

	return 0;
}
