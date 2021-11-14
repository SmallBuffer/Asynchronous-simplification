#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>

struct Server{
	int id;
}P;
struct Client{
        int ack;
}Q1,Q2,Q3,Q4,Q5;

void* Thread_1()
{

       __VERIFIER_atomic_begin();

        P.id = 1;
        Q1.ack = 1;
        
	__VERIFIER_atomic_end();

        assert(Q1.ack==1);
        return 0;
}
void* Thread_2()
{

       __VERIFIER_atomic_begin();

        P.id = 1;
        Q2.ack = 1;
     
	__VERIFIER_atomic_end();


        assert(Q2.ack==1);
        return 0;
}
void* Thread_3()
{

       __VERIFIER_atomic_begin();

	P.id = 1;
	Q3.ack = 1;

	__VERIFIER_atomic_end();


	assert(Q3.ack==1);
	return 0;
}
void* Thread_4()
{

       __VERIFIER_atomic_begin();

	P.id = 1;
	Q4.ack = 1;

	__VERIFIER_atomic_end();


	assert(Q4.ack==1);
	return 0;
}
void* Thread_5()
{

       __VERIFIER_atomic_begin();


	P.id = 1;
	Q5.ack = 1;

	__VERIFIER_atomic_end();


	assert(Q5.ack==1);
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
