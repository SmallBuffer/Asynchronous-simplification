#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
struct Mes{
	int message;
	int round;
};
struct Server{
	struct Mes server_send,server_recv;
}P;
struct Client{
	int v[100];
	struct Mes client_recv,client_send;
	int rid;
}Q1,Q2;
int r;

bool mutex1 = false;

void* Thread_1(void * par)
{
	__VERIFIER_atomic_begin();

	P.server_send.message = 1;
	P.server_send.round = 1;
	Q1.client_recv = P.server_send;
	Q1.rid = Q1.client_recv.round;;
	Q1.v[Q1.rid] = Q1.client_recv.message;;
	Q1.client_send.message = 2;
	Q1.client_send.round = Q1.rid;
	P.server_recv = Q1.client_send;
	assert(P.server_recv.message == 2);

	__VERIFIER_atomic_end();

	__VERIFIER_atomic_begin();

	P.server_send.message = 1;
	P.server_send.round = 2;
	Q1.client_recv = P.server_send;
	Q1.rid = Q1.client_recv.round;;
	Q1.v[Q1.rid] = Q1.client_recv.message;;
	Q1.client_send.message = 2;
	Q1.client_send.round = Q1.rid;
	P.server_recv = Q1.client_send;
	assert(P.server_recv.message == 2);

	__VERIFIER_atomic_end();
	return 0;
}
void* Thread_2(void * par)
{

	__VERIFIER_atomic_begin();

	P.server_send.message = 1;
	P.server_send.round = 1;
	Q2.client_recv = P.server_send;
	Q2.rid = Q2.client_recv.round;;
	Q2.v[Q1.rid] = Q2.client_recv.message;;
	Q2.client_send.message = 2;
	Q2.client_send.round = Q2.rid;
	P.server_recv = Q2.client_send;
	assert(P.server_recv.message == 2);

	__VERIFIER_atomic_end();

	__VERIFIER_atomic_begin();

	P.server_send.message = 1;
	P.server_send.round = 2;
	Q2.client_recv = P.server_send;
	Q2.rid = Q2.client_recv.round;;
	Q2.v[Q1.rid] = Q2.client_recv.message;;
	Q2.client_send.message = 2;
	Q2.client_send.round = Q2.rid;
	P.server_recv = Q2.client_send;
	assert(P.server_recv.message == 2);

	__VERIFIER_atomic_end();
	return 0;
}
int main()
{

	for(int r =0;r<2;++r)
	{
		pthread_t thread[3];

		pthread_create(&thread[1], NULL, Thread_1, NULL);
		pthread_create(&thread[2], NULL, Thread_2, NULL);

		for( int i = 1; i <= 2; i++)
			pthread_join(thread[i], NULL); 

	}

	return EXIT_SUCCESS;
}

/* 
   1 => ping
   2 => pong
   */
