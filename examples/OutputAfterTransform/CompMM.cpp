#include <assert.h>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include<vector>
using namespace std;
struct Server{
        string bufRecv;
}P[100];
struct Client{
        string buffer;
}Q[100];
struct Pair{
        int id_p;
        int id_q;
};

bool mutex1 = false;

void* Thread(void * par)
{
        int id_p = ((Pair *)par)->id_p;
        int id_q = ((Pair *)par)->id_q;

        while (mutex1 == true);
        mutex1 = true;

        Q[id_q].buffer = "ping";
        P[id_p].bufRecv = "pong";
        assert(P[id_p].bufRecv == "pong");
        mutex1 = false;

        return 0;
}

int main()
{

        for( int i ; i < 100 ; ++i)
        {

                for( int j ; j < 100 ; ++j)
                {

                        pthread_t id;
                        int ret = 0;
                        Pair par;

                        par.id_p = i;
                        par.id_q = j;
                        ret = pthread_create(&id, NULL, Thread, &par);
                        pthread_join(id, NULL);

                }

        }

        return 0;
}