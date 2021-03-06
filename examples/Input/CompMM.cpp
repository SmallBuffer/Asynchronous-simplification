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

void Send(std::queue<int>* addr,int x)
{
	addr->push(x);
}
void Recv(std::queue<int>* addr,int* res )
{
	while(addr->empty());	//缓存区为空时读取阻塞
	*res = addr->front();
	addr->pop();
}

void* Client_Thread(void *par) // participator-client
{
	int id = *((int *)par);
	int message_recv;
	for (int i = 0; i < server_num;i++) // Iterate over multiple-server
	{
		Recv(&client_buffer[id], &message_recv);
		Send(&server_buffer[i],2); 
	}

	return 0;
}
void * Server_Thread(void *par) // participator-server
{
	int id = *((int *)par);
	int message_recv;
	for (int i = 0; i < client_num;++i) // Iterate over multiple-client
	{
		Send(&client_buffer[i], 1);		
		Recv(&server_buffer[id], &message_recv);		
		assert(message_recv == 2);
	}
	return 0;
}
int main()
{
	//start thread
	for(int i=0;i<server_num;++i){
		pthread_t id;
		int ret = 0;

		ret = pthread_create(&id, NULL,Server_Thread,&i);
		pthread_join(id,NULL);
	}

	for( int i=0 ; i < client_num ; ++i)
	{

		pthread_t id;
		int ret = 0;

		ret = pthread_create(&id, NULL,Client_Thread,&i);
		pthread_join(id,NULL);

	}

}
/*
   1 => ping
   2 => pong
   */
