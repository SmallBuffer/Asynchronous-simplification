#include <assert.h>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
struct Server{
        string bufRecv;
}P;
struct Client{
        string buffer;
}Q[100];
bool mutex1 = false;

void* Thread(void * par)
{
        int id_q = *((int *)par);

        while (mutex1 == true);
        mutex1 = true;

        Q[id_q].buffer = "ping";
        P.bufRecv = "pong";
        assert(P.bufRecv == "pong");
        mutex1 = false;

        return 0;
}

int main()
{

        for( int i ; i < 100 ; ++i)
        {

                pthread_t id;
                int ret = 0;

                ret = pthread_create(&id, NULL, Thread, &i);
                pthread_join(id,NULL);

        }

        return 0;
}