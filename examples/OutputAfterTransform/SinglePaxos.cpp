#include <assert.h>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include<stdbool.h>
#include<string>
using namespace std;

struct PROPOSAL
{
	int	serialNum;
	int	value;
};
struct Server{
    unsigned int N,value, accepted_num;
    unsigned int temp_serialNum;
    int agree_num;
    bool is_commited;
    PROPOSAL proposal_recv, proposal_send;
}P[100];
struct Client{
    unsigned int N,resN,acceptN,acceptV;
    PROPOSAL proposal_recv, proposal_send;
    bool is_commited;
} Q[100];
struct Pair{
        int i;
        int j;
};

bool mutex1 = false;

bool mutex2 = false;

int i, j;


void* Thread1(void *)
{

    while (mutex1 == true);
        mutex1 = true;

    Q[j].N = P[i].N;
    if(Q[j].N>Q[j].resN)
    {
        Q[j].resN = Q[j].N;
        Q[j].proposal_send.serialNum = Q[j].acceptN;
        Q[j].proposal_send.value = Q[j].acceptV;
    }
    else
    {
        Q[j].proposal_send.serialNum = -1 ;
        Q[j].proposal_send.value = -1;
    }
    P[i].proposal_recv = Q[j].proposal_send;
    if(P[i].proposal_recv.serialNum!=-1&&P[i].proposal_recv.value!=-1)
    {
        if(P[i].proposal_recv.serialNum>P[i].temp_serialNum)
        {
            P[i].value = P[i].proposal_recv.value;
            P[i].temp_serialNum = P[i].proposal_recv.serialNum;
        }
        P[i].agree_num++;
    }   

    mutex1 = false;

    return 0;
}
void* Thread2(void *)
{

    while (mutex2 == true);
        mutex2 = true;

    Q[i].proposal_recv = P[i].proposal_send;
    if(Q[j].proposal_recv.serialNum>Q[j].resN)
    {
        Q[j].resN = Q[j].proposal_recv.serialNum;
        Q[j].acceptN = Q[j].proposal_recv.serialNum;
        Q[j].acceptV = Q[j].proposal_recv.value;
        Q[j].is_commited = true;
    }
    else
    {
        Q[j].is_commited = false;
    }

    P[i].is_commited = Q[j].is_commited;

    if(P[i].is_commited)
        P[i].agree_num = P[i].agree_num + 1;
    
    mutex2 = false;

    return 0;
}
int main()
{
    for(i ; i < 100 ; ++i)
    {
        while(true)
        {
            P[i].N = time(0)%10000;
            while (true)
            {   
                P[i].agree_num = 0;
                P[i].temp_serialNum = 0;
                P[i].N = P[i].N + 5;

                for (j = 0; j < 100;j++)
                {
                     pthread_t id;
                    int ret = 0;
                   
                    ret = pthread_create(&id, NULL, Thread1,NULL);
                    pthread_join(id, NULL);
                }

                if (2 * P[i].agree_num > 100)
                    break;
            }
            P[i].proposal_send.serialNum =P[i]. N;
            P[i].proposal_send.value = P[i].value;
            P[i].agree_num = 0;
            
            for (int j = 0; j < 100;j++)
            {
                pthread_t id;
                int ret = 0;
        
                ret = pthread_create(&id, NULL, Thread2,NULL);
                pthread_join(id, NULL);
            }
            

            if( 2 * P[i].agree_num > 100)break;
        }

    }
    return 0;
}
