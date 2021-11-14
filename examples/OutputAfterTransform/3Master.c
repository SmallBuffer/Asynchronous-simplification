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
        int id;
	int message_recv;
}Q1,Q2,Q3,Q4,Q5;
struct Master{
	int message_recv;
}M;

void* Thread_1()
{
	Q1.id = 1;

        __VERIFIER_atomic_begin();

	P.id = Q1.id;
        Q1.message_recv = 2;

        __VERIFIER_atomic_end();

        assert(Q1.message_recv==2);

	__VERIFIER_atomic_begin();

	M.message_recv = 1;
	assert(M.message_recv == 1);

	__VERIFIER_atomic_end();

        return 0;
}
void* Thread_2()
{
	Q2.id = 2;

        __VERIFIER_atomic_begin();

	P.id = Q2.id;
        Q2.message_recv = 2;

        __VERIFIER_atomic_end();

        assert(Q2.message_recv==2);

	__VERIFIER_atomic_begin();

	M.message_recv = 1;
	assert(M.message_recv == 1);

	__VERIFIER_atomic_end();

        return 0;
}
void* Thread_3()
{
	Q3.id = 1;

        __VERIFIER_atomic_begin();

	P.id = Q3.id;
        Q3.message_recv = 2;

        __VERIFIER_atomic_end();

        assert(Q3.message_recv==2);

	__VERIFIER_atomic_begin();

	M.message_recv = 1;
	assert(M.message_recv == 1);

	__VERIFIER_atomic_end();

        return 0;
}
void* Thread_4()
{
	Q4.id = 1;

        __VERIFIER_atomic_begin();

	P.id = Q4.id;
        Q4.message_recv = 2;

        __VERIFIER_atomic_end();

        assert(Q4.message_recv==2);

	__VERIFIER_atomic_begin();

	M.message_recv = 1;
	assert(M.message_recv == 1);

	__VERIFIER_atomic_end();

        return 0;
}
void* Thread_5()
{
	Q5.id = 1;

        __VERIFIER_atomic_begin();

	P.id = Q5.id;
        Q5.message_recv = 2;

        __VERIFIER_atomic_end();

        assert(Q5.message_recv==2);

	__VERIFIER_atomic_begin();

	M.message_recv = 1;
	assert(M.message_recv == 1);

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
