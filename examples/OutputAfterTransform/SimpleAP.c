#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
struct Server{
	int message_recv;
}P;
struct Client{
	int message_recv;
}Q1,Q2,Q3,Q4,Q5;

void* Thread_1()
{
	__VERIFIER_atomic_begin();

	Q1.message_recv = 1; 
	P.message_recv = 2;
	assert(P.message_recv == 2);

	__VERIFIER_atomic_end();

	return 0;
}
void* Thread_2()
{

	__VERIFIER_atomic_begin();
	Q2.message_recv = 1; 
	P.message_recv = 2;
	assert(P.message_recv == 2);
	__VERIFIER_atomic_end();

	return 0;
}
void* Thread_3()
{

	__VERIFIER_atomic_begin();
	Q3.message_recv = 1; 
	P.message_recv = 2;
	assert(P.message_recv == 2);

	__VERIFIER_atomic_end();


	return 0;
}
void* Thread_4()
{


	__VERIFIER_atomic_begin();

	Q4.message_recv = 1; 
	P.message_recv = 2;
	assert(P.message_recv == 2);

	__VERIFIER_atomic_end();


	return 0;
}
void* Thread_5()
{


	__VERIFIER_atomic_begin();

	Q5.message_recv = 1; 
	P.message_recv = 2;
	assert(P.message_recv == 2);

	__VERIFIER_atomic_end();


	return 0;
}


int main()
{

	pthread_t thread[6];

	pthread_create(&thread[1], NULL, Thread_1, NULL);
	pthread_create(&thread[2], NULL, Thread_2, NULL);
	pthread_create(&thread[3], NULL, Thread_3, NULL);
	pthread_create(&thread[4], NULL, Thread_4, NULL);
	pthread_create(&thread[5], NULL, Thread_5, NULL);

	for( int i = 1; i <= 5; i++)
		pthread_join(thread[i], NULL);  


	return EXIT_SUCCESS;


}

/* 
   1 => ping
   2 => pong
   */
