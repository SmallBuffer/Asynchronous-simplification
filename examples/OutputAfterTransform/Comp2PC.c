#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

struct Mes
{
    int message;
    int c;
};
struct Server{
        int  server_msg;
        int prop;
        int reply;
        struct Mes mess_send;
        int ack;
        int committed ;
	int is_aborted; 
}P;
struct Client{
        int client_msg;
        int decision;
        int id;
        struct Mes mess_recv;
        int val;
        int value;
} Q1,Q2;

void* Thread_1()
{
        __VERIFIER_atomic_begin();

        P.mess_send.c = 1;
        P.mess_send.message = P.prop;
        Q1.mess_recv = P.mess_send;

        __VERIFIER_atomic_end();

        Q1.id = Q1.mess_recv.c;;
        Q1.val = Q1.mess_recv.message;;
        if (rand()%2) //用随机数模拟是否确认
        {
        Q1.client_msg = 1;
        }
        else
        {
        Q1.client_msg = 0;
        };
        
        __VERIFIER_atomic_begin();

        P.server_msg = Q1.client_msg;
        if(P.server_msg == 0){
        P.is_aborted = 1;
        }
        
        __VERIFIER_atomic_end();


        if(P.is_aborted ==0)
        {
        P.reply = 1;
        P.committed = 1;
        }
        else{
        P.reply = 0;
        }

        __VERIFIER_atomic_begin();

        Q1.decision = P.reply;

         __VERIFIER_atomic_end();

        if(Q1.decision==1){
        Q1.value = Q1.val;
        }
        
        __VERIFIER_atomic_begin();

        P.ack = 1;
       assert(P.ack == 1);

         __VERIFIER_atomic_end();


        return 0;
}

void* Thread_2()
{
        __VERIFIER_atomic_begin();

        P.mess_send.c = 1;
        P.mess_send.message = P.prop;
        Q2.mess_recv = P.mess_send;

        __VERIFIER_atomic_end();

        Q2.id = Q2.mess_recv.c;;
        Q2.val = Q2.mess_recv.message;;
        if (rand()%2) //用随机数模拟是否确认
        {
        Q2.client_msg = 1;
        }
        else
        {
        Q2.client_msg = 0;
        };
        
        __VERIFIER_atomic_begin();

        P.server_msg = Q2.client_msg;
        if(P.server_msg == 0){
        P.is_aborted = 1;
        }
        
        __VERIFIER_atomic_end();


        if(P.is_aborted ==0)
        {
        P.reply = 1;
        P.committed = 1;
        }
        else{
        P.reply = 0;
        }

        __VERIFIER_atomic_begin();

        Q2.decision = P.reply;

         __VERIFIER_atomic_end();

        if(Q2.decision==1){
        Q2.value = Q2.val;
        }
        
        __VERIFIER_atomic_begin();

        P.ack = 1;
       assert(P.ack == 1);

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
   Commit => 1
   Abort => 0
   */
