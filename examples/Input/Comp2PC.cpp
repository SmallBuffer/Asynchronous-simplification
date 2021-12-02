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

std::queue<int> server_buffer,client_buffer[client_num];

struct Mes{
	int message;
	int c;
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

void * Client_Thread(void *par) // participator-client
{
	struct Mes mess_recv;
	int id_p,id_q,val,client_msg,decision,value;
	id_q = *((int *)par);
	Recv(&client_buffer[id_q],&mess_recv,sizeof(struct Mes));
	id_p = mess_recv.c;
	val = mess_recv.message;
	if (rand()%2) //用随机数模拟是否确认
	{
		client_msg = 1;
	}
	else
	{
		client_msg=0;
	};
	Send(&server_buffer, &client_msg, sizeof(int));
	Recv(&client_buffer[id_q],&decision, sizeof(int));
	if(decision==1){
		value = val;
	}
	int ack = 1;
	Send(&server_buffer, &ack, sizeof(int));
	return 0;
}

void * Server_Thread(void * par)  // participator-server
{

	struct Mes mess_send;
	int prop,server_msg,is_aborted,reply,committed,ack;
	for (int i = 0; i < client_num; ++i) // Iterate over multiple-client
	{
		mess_send.c = i;
		mess_send.message=prop;
		Send(&client_buffer[i],&mess_send,sizeof(struct Mes));
	}
	for (int i = 0; i < client_num;++i) // Iterate over multiple-client
	{
		Recv(&server_buffer,&server_msg,sizeof(int));
		if(server_msg == 0){
			is_aborted = 1;
		}
	}
	if(is_aborted == 0)
	{
		reply = 1;
		committed = 1;
	}
	else{
		reply = 0;
	}
	for (int i = 0; i < client_num;++i) // Iterate over multiple-client
	{
		Send(&client_buffer[i],&reply,sizeof(int));
	}
	for (int i = 0; i < client_num; ++i) // Iterate over multiple-client
	{
		Recv(&server_buffer,&ack,sizeof(int));
		assert(ack == 1);
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
   Commit => 1
   Abort => 0
   */
