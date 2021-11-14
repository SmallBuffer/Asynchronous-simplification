#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>

#define client_num  2
#define server_num  2
struct PROPOSAL
{
	int	serialNum;
	int	value;
	int	is_agreed;
	int	type;
};
struct Server{
	unsigned int N,value, accepted_num;
	unsigned int temp_serialNum;
	int agree_num;
	bool is_commited;
	struct PROPOSAL proposal_recv, proposal_send;
}P1,P2;
struct Client{
	unsigned int N,resN,acceptN,acceptV;
	struct PROPOSAL proposal_recv, proposal_send;
	bool is_commited;
} Q1,Q2;

int cnt1 = 0;

void* Thread_1( )
{
	while(true)
	{
		P1.N = time(0)%10000;
		while (true)
		{   

			P1.agree_num = 0;
			P1.temp_serialNum = 0;
			P1.N = P1.N + 5;
			__VERIFIER_atomic_begin();
			Q1.proposal_recv = P1.proposal_send;
			if(Q1.proposal_recv.type == 0)
			{
				Q1.N = Q1.proposal_recv.serialNum;
				if(Q1.N>Q1.resN)
				{
					Q1.resN = Q1.N;
					Q1.proposal_send.serialNum = Q1.acceptN;
					Q1.proposal_send.value = Q1.acceptV;
				}
				else
				{
					Q1.proposal_send.is_agreed = 0;
				}
			}
			else if(Q1.proposal_recv.type == 1)
			{
				if(Q1.proposal_recv.serialNum>Q1.resN)
				{
					Q1.resN = Q1.proposal_recv.serialNum;
					Q1.acceptN = Q1.proposal_recv.serialNum;
					Q1.acceptV = Q1.proposal_recv.value;
					Q1.proposal_send.is_agreed = 1;
				}
				else
				{
					Q1.proposal_send.is_agreed = 0;
				}
			}
			__VERIFIER_atomic_end();

			__VERIFIER_atomic_begin();
			P1.proposal_recv = Q1.proposal_send;
			if(P1.proposal_recv.is_agreed == 1)
			{
				if(P1.proposal_recv.serialNum>P1.temp_serialNum)
				{
					P1.value = P1.proposal_recv.value;
					P1.temp_serialNum = P1.proposal_recv.serialNum;
				}
				P1.agree_num++;
			}    
			__VERIFIER_atomic_end();

			if( 2 * P1.agree_num > client_num)break;
		}
		P1.proposal_send.serialNum = P1.N;
		P1.proposal_send.value = P1.value;
		P1.proposal_send.type = 1;
		P1.agree_num = 0;

		__VERIFIER_atomic_begin();
		Q1.proposal_recv = P1.proposal_send;
		if(Q1.proposal_recv.type == 0)
		{
			Q1.N = Q1.proposal_recv.serialNum;
			if(Q1.N>Q1.resN)
			{
				Q1.resN = Q1.N;
				Q1.proposal_send.serialNum = Q1.acceptN;
				Q1.proposal_send.value = Q1.acceptV;
			}
			else
			{
				Q1.proposal_send.is_agreed = 0;
			}
		}
		else if(Q1.proposal_recv.type == 1)
		{
			if(Q1.proposal_recv.serialNum>Q1.resN)
			{
				Q1.resN = Q1.proposal_recv.serialNum;
				Q1.acceptN = Q1.proposal_recv.serialNum;
				Q1.acceptV = Q1.proposal_recv.value;
				Q1.proposal_send.is_agreed = 1;
			}
			else
			{
				Q1.proposal_send.is_agreed = 0;
			}
		}
		__VERIFIER_atomic_end();
		__VERIFIER_atomic_begin();
		P1.proposal_recv = Q1.proposal_send;
		if(P1.proposal_recv.is_agreed)
			P1.agree_num = P1.agree_num + 1;
		__VERIFIER_atomic_end();
		if( 2 * P1.agree_num > client_num)break;
	}
	assert(P1.value == server_num);

	cnt1++;
	while(cnt1<2) ;//wait for all receivers receiving their messages 

	while(true)
	{
		P2.N = time(0)%10000;
		while (true)
		{   

			P2.agree_num = 0;
			P2.temp_serialNum = 0;
			P2.N = P2.N + 5;
			__VERIFIER_atomic_begin();
			Q1.proposal_recv = P2.proposal_send;
			if(Q1.proposal_recv.type == 0)
			{
				Q1.N = Q1.proposal_recv.serialNum;
				if(Q1.N>Q1.resN)
				{
					Q1.resN = Q1.N;
					Q1.proposal_send.serialNum = Q1.acceptN;
					Q1.proposal_send.value = Q1.acceptV;
				}
				else
				{
					Q1.proposal_send.is_agreed = 0;
				}
			}
			else if(Q1.proposal_recv.type == 1)
			{
				if(Q1.proposal_recv.serialNum>Q1.resN)
				{
					Q1.resN = Q1.proposal_recv.serialNum;
					Q1.acceptN = Q1.proposal_recv.serialNum;
					Q1.acceptV = Q1.proposal_recv.value;
					Q1.proposal_send.is_agreed = 1;
				}
				else
				{
					Q1.proposal_send.is_agreed = 0;
				}
			}
			__VERIFIER_atomic_end();

			P2.proposal_recv = Q1.proposal_send;
			if(P2.proposal_recv.is_agreed == 1)
			{
				if(P2.proposal_recv.serialNum>P2.temp_serialNum)
				{
					P2.value = P2.proposal_recv.value;
					P2.temp_serialNum = P2.proposal_recv.serialNum;
				}
				P2.agree_num++;
			}    

			if( 2 * P2.agree_num > client_num)break;
		}
		P2.proposal_send.serialNum = P2.N;
		P2.proposal_send.value = P2.value;
		P2.proposal_send.type = 1;
		P2.agree_num = 0;

		__VERIFIER_atomic_begin();
		Q1.proposal_recv = P2.proposal_send;
		if(Q1.proposal_recv.type == 0)
		{
			Q1.N = Q1.proposal_recv.serialNum;
			if(Q1.N>Q1.resN)
			{
				Q1.resN = Q1.N;
				Q1.proposal_send.serialNum = Q1.acceptN;
				Q1.proposal_send.value = Q1.acceptV;
			}
			else
			{
				Q1.proposal_send.is_agreed = 0;
			}
		}
		else if(Q1.proposal_recv.type == 1)
		{
			if(Q1.proposal_recv.serialNum>Q1.resN)
			{
				Q1.resN = Q1.proposal_recv.serialNum;
				Q1.acceptN = Q1.proposal_recv.serialNum;
				Q1.acceptV = Q1.proposal_recv.value;
				Q1.proposal_send.is_agreed = 1;
			}
			else
			{
				Q1.proposal_send.is_agreed = 0;
			}
		}
		__VERIFIER_atomic_end();
		__VERIFIER_atomic_begin();
		P2.proposal_recv = Q1.proposal_send;
		if(P2.proposal_recv.is_agreed)
			P2.agree_num = P2.agree_num + 1;
		__VERIFIER_atomic_end();
		if( 2 * P2.agree_num > client_num)break;
	}
	assert(P2.value == server_num); 

	return 0;
}
void* Thread_2( )
{
	while(true)
	{
		P1.N = time(0)%10000;
		while (true)
		{   

			P1.agree_num = 0;
			P1.temp_serialNum = 0;
			P1.N = P1.N + 5;
			__VERIFIER_atomic_begin();
			Q2.proposal_recv = P1.proposal_send;
			if(Q2.proposal_recv.type == 0)
			{
				Q2.N = Q2.proposal_recv.serialNum;
				if(Q2.N>Q2.resN)
				{
					Q2.resN = Q2.N;
					Q2.proposal_send.serialNum = Q2.acceptN;
					Q2.proposal_send.value = Q2.acceptV;
				}
				else
				{
					Q2.proposal_send.is_agreed = 0;
				}
			}
			else if(Q2.proposal_recv.type == 1)
			{
				if(Q2.proposal_recv.serialNum>Q2.resN)
				{
					Q2.resN = Q2.proposal_recv.serialNum;
					Q2.acceptN = Q2.proposal_recv.serialNum;
					Q2.acceptV = Q2.proposal_recv.value;
					Q2.proposal_send.is_agreed = 1;
				}
				else
				{
					Q2.proposal_send.is_agreed = 0;
				}
			}
			__VERIFIER_atomic_end();

			__VERIFIER_atomic_begin();
			P1.proposal_recv = Q2.proposal_send;
			if(P1.proposal_recv.is_agreed == 1)
			{
				if(P1.proposal_recv.serialNum>P1.temp_serialNum)
				{
					P1.value = P1.proposal_recv.value;
					P1.temp_serialNum = P1.proposal_recv.serialNum;
				}
				P1.agree_num++;
			}    
			__VERIFIER_atomic_end();

			if( 2 * P1.agree_num > client_num)break;
		}
		P1.proposal_send.serialNum = P1.N;
		P1.proposal_send.value = P1.value;
		P1.proposal_send.type = 1;
		P1.agree_num = 0;

		__VERIFIER_atomic_begin();
		Q2.proposal_recv = P1.proposal_send;
		if(Q2.proposal_recv.type == 0)
		{
			Q2.N = Q2.proposal_recv.serialNum;
			if(Q2.N>Q2.resN)
			{
				Q2.resN = Q2.N;
				Q2.proposal_send.serialNum = Q2.acceptN;
				Q2.proposal_send.value = Q2.acceptV;
			}
			else
			{
				Q2.proposal_send.is_agreed = 0;
			}
		}
		else if(Q2.proposal_recv.type == 1)
		{
			if(Q2.proposal_recv.serialNum>Q2.resN)
			{
				Q2.resN = Q2.proposal_recv.serialNum;
				Q2.acceptN = Q2.proposal_recv.serialNum;
				Q2.acceptV = Q2.proposal_recv.value;
				Q2.proposal_send.is_agreed = 1;
			}
			else
			{
				Q2.proposal_send.is_agreed = 0;
			}
		}
		__VERIFIER_atomic_end();
		__VERIFIER_atomic_begin();
		P1.proposal_recv = Q2.proposal_send;
		if(P1.proposal_recv.is_agreed)
			P1.agree_num = P1.agree_num + 1;
		__VERIFIER_atomic_end();
		if( 2 * P1.agree_num > client_num)break;
	}
	assert(P1.value == server_num); 
	cnt1++;
	while(cnt1<2) ;//wait for all receivers receiving their messages 

	while(true)
	{
		P2.N = time(0)%10000;
		while (true)
		{   

			P2.agree_num = 0;
			P2.temp_serialNum = 0;
			P2.N = P2.N + 5;
			__VERIFIER_atomic_begin();
			Q2.proposal_recv = P2.proposal_send;
			if(Q2.proposal_recv.type == 0)
			{
				Q2.N = Q2.proposal_recv.serialNum;
				if(Q2.N>Q2.resN)
				{
					Q2.resN = Q2.N;
					Q2.proposal_send.serialNum = Q2.acceptN;
					Q2.proposal_send.value = Q2.acceptV;
				}
				else
				{
					Q2.proposal_send.is_agreed = 0;
				}
			}
			else if(Q2.proposal_recv.type == 1)
			{
				if(Q2.proposal_recv.serialNum>Q2.resN)
				{
					Q2.resN = Q2.proposal_recv.serialNum;
					Q2.acceptN = Q2.proposal_recv.serialNum;
					Q2.acceptV = Q2.proposal_recv.value;
					Q2.proposal_send.is_agreed = 1;
				}
				else
				{
					Q2.proposal_send.is_agreed = 0;
				}
			}
			__VERIFIER_atomic_end();

			P2.proposal_recv = Q2.proposal_send;
			if(P2.proposal_recv.is_agreed == 1)
			{
				if(P2.proposal_recv.serialNum>P2.temp_serialNum)
				{
					P2.value = P2.proposal_recv.value;
					P2.temp_serialNum = P2.proposal_recv.serialNum;
				}
				P2.agree_num++;
			}    

			if( 2 * P2.agree_num > client_num)break;
		}
		P2.proposal_send.serialNum = P2.N;
		P2.proposal_send.value = P2.value;
		P2.proposal_send.type = 1;
		P2.agree_num = 0;

		__VERIFIER_atomic_begin();
		Q2.proposal_recv = P2.proposal_send;
		if(Q2.proposal_recv.type == 0)
		{
			Q2.N = Q2.proposal_recv.serialNum;
			if(Q2.N>Q2.resN)
			{
				Q2.resN = Q2.N;
				Q2.proposal_send.serialNum = Q2.acceptN;
				Q2.proposal_send.value = Q2.acceptV;
			}
			else
			{
				Q2.proposal_send.is_agreed = 0;
			}
		}
		else if(Q2.proposal_recv.type == 1)
		{
			if(Q2.proposal_recv.serialNum>Q2.resN)
			{
				Q2.resN = Q2.proposal_recv.serialNum;
				Q2.acceptN = Q2.proposal_recv.serialNum;
				Q2.acceptV = Q2.proposal_recv.value;
				Q2.proposal_send.is_agreed = 1;
			}
			else
			{
				Q2.proposal_send.is_agreed = 0;
			}
		}
		__VERIFIER_atomic_end();
		__VERIFIER_atomic_begin();
		P2.proposal_recv = Q2.proposal_send;
		if(P2.proposal_recv.is_agreed)
			P2.agree_num = P2.agree_num + 1;
		__VERIFIER_atomic_end();
		if( 2 * P2.agree_num > client_num)break;
	}
	assert(P2.value == server_num); 

	return 0;
}

int main()
{
	pthread_t thread[3];


	pthread_create(&thread[1], NULL, Thread_1, NULL);
	pthread_create(&thread[2], NULL, Thread_2, NULL);

	for( int i = 1; i <= 2; i++)
		pthread_join(thread[i], NULL);  
	return 0;
}
