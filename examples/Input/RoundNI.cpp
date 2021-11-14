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
#define server_num 1
#define Round 20

std::queue<int> server_buffer,client_buffer[client_num];

struct Mes{
	int message;
	int round;
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

void *Client_Thread(void * par)
{
	int v[Round];
	struct Mes client_recv,client_send;
	int rid;
	for (int i = 0; i < Round; ++i) //repeat_loop
	{
	
		Send(&server_buffer, &client_recv, sizeof(struct Mes));
		rid = client_recv.round;
		v[rid] = client_recv.message;
		client_send.message=2;
		client_send.round = rid;
		Send(&server_buffer, &client_send, sizeof(struct Mes));
	}

	return 0;
}

void * Server_Thread(void * par) {

	struct Mes server_send,server_recv;
	for(int r=0;r<Round;++r)	//repeat_loop
	{
		for (int i = 0; i < client_num;++i)
		{
			server_send.message=1;
			server_send.round = r;
			Send(&client_buffer[i], &server_send, sizeof(struct Mes));
			Recv(&server_buffer,&server_recv, sizeof(struct Mes));
			assert(server_recv.message == 2);
		}
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

		ret = pthread_create(&id, NULL,Client_Thread,&i);
		pthread_join(id,NULL);

	}

}
/* 
   1 => ping
   2 => pong
   */
