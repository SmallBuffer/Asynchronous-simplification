#include <unistd.h>
#include <queue>
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#define client_num 100
#define server_num 1
#define master_num 1

std::queue<int> server_buffer,master_buffer,client_buffer[client_num];

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

void * Client_Thread(void *par)
{
	int id,message_recv;
	id = *((int *)par);
	Send(&server_buffer,id);
	Recv(&client_buffer[id], &message_recv);//read from server
	assert(message_recv==2);
	Send(&master_buffer,1); //send to master

	return 0;
}

void * Server_Thread(void * par)
{
	int id ;
	for (int i = 0; i < client_num;++i) //communicate with clients
	{
		Recv(&server_buffer, &id);
		Send(&client_buffer[i], 2);
	}

	return 0;
}

void * Master_Thread(void * par)
{
	int message_recv;
	for (int i = 0; i < client_num;++i) //communicate with clients
	{
		Recv(&master_buffer, &message_recv);	
		assert(message_recv == 1);
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
	for(int i=0;i<master_num;++i){
		pthread_t id;
		int ret = 0;

		ret = pthread_create(&id, NULL,Master_Thread,NULL);
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
/*
   1 => pong
   2 => ping
   3 => ack
   */
