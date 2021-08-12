#include <assert.h>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include<string>
using namespace std;
struct Mes{
    string message;
    int round;
};
struct Server{
        Mes server_send,server_recv;
}P;
struct Client{
        string v[100];
        Mes client_recv,client_send;
        int rid;
}Q[100];
int r;

bool mutex1 = false;

void* Thread(void * par)
{
        int id_q = *((int *)par);

        while (mutex1 == true);
        mutex1 = true;

        P.server_send.message = "ping";
        P.server_send.round = r;
        Q[id_q].client_recv = P.server_send;
        Q[id_q].rid = Q[id_q].client_recv.round;;
        Q[id_q].v[Q[id_q].rid] = Q[id_q].client_recv.message;;
        Q[id_q].client_send.message = "pong";
        Q[id_q].client_send.round = Q[id_q].rid;
        P.server_recv = Q[id_q].client_send;
        assert(P.server_recv.message == "pong");
        mutex1 = false;

        return 0;
}

int main()
{

        for( r=0 ; r < 100 ; ++r)
        {

                for( int i ; i < 100 ; ++i)
                {

                        pthread_t id;
                        int ret = 0;

                        ret = pthread_create(&id, NULL, Thread, &i);
                        pthread_join(id,NULL);

                }

        }

        return 0;
}