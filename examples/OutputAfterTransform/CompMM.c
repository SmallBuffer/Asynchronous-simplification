#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
struct Server{
	int message_recv;
}P1,P2;
struct Client{
	int message_recv;
}Q1,Q2;
struct Pair{
	int id_p;
	int id_q;
};

void* Thread_1()
{
        __VERIFIER_atomic_begin();

	Q1.message_recv = 1;
	P1.message_recv = 2;
	assert(P1.message_recv == 2);

	 __VERIFIER_atomic_end();

        __VERIFIER_atomic_begin();

	Q1.message_recv = 1;
	P2.message_recv = 2;
	assert(P2.message_recv == 2);

	 __VERIFIER_atomic_end();

	return 0;
}
void* Thread_2()
{
        __VERIFIER_atomic_begin();

	Q2.message_recv = 1;
	P1.message_recv = 2;
	assert(P1.message_recv == 2);

 	__VERIFIER_atomic_end();
	
        __VERIFIER_atomic_begin();

	Q2.message_recv = 1;
	P2.message_recv = 2;
	assert(P2.message_recv == 2);

 	__VERIFIER_atomic_end();

	return 0;
}

int main()
{

	pthread_t thread[3];


	pthread_create(&thread[1], NULL, Thread_1, NULL);
	pthread_create(&thread[2], NULL, Thread_2, NULL);


	for( int i = 1; i <= 2; i++)
		pthread_join(thread[i], NULL);  


	return EXIT_SUCCESS;
}

/* 
   1 => ping
   2 => pong
   */
