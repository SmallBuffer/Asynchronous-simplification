#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
struct SERVER{
		int id;
}server1;

struct CLIENT{
		int ack;
		int id;
}client1,client2,client3,client4,client5;

void* Thread_1(void * par){
	client1.id = *((int *)par);
	__VERIFIER_atomic_begin();
	server1.id = client1.id;
	client1.ack = 1;
	__VERIFIER_atomic_end();
	return 0;
}
void* Thread_2(void * par){
	client2.id = *((int *)par);
	__VERIFIER_atomic_begin();
	server1.id = client2.id;
	client2.ack = 1;
	__VERIFIER_atomic_end();
	return 0;
}
void* Thread_3(void * par){
	client3.id = *((int *)par);
	__VERIFIER_atomic_begin();
	server1.id = client3.id;
	client3.ack = 1;
	__VERIFIER_atomic_end();
	return 0;
}
void* Thread_4(void * par){
	client4.id = *((int *)par);
	__VERIFIER_atomic_begin();
	server1.id = client4.id;
	client4.ack = 1;
	__VERIFIER_atomic_end();
	return 0;
}
void* Thread_5(void * par){
	client5.id = *((int *)par);
	__VERIFIER_atomic_begin();
	server1.id = client5.id;
	client5.ack = 1;
	__VERIFIER_atomic_end();
	return 0;
}
int main(){
	pthread_t thread[5];
	pthread_create(&thread[1], NULL, Thread_1, NULL);
	pthread_create(&thread[2], NULL, Thread_2, NULL);
	pthread_create(&thread[3], NULL, Thread_3, NULL);
	pthread_create(&thread[4], NULL, Thread_4, NULL);
	pthread_create(&thread[5], NULL, Thread_5, NULL);
	for( int i = 1; i <= 5; i++)
		pthread_join(thread[i], &i);
	return EXIT_SUCCESS;
}
