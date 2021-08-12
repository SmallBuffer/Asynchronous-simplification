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

struct Mes
{
    string message;
    int c;
};
struct Server{
        string server_msg;
        string prop;
        string reply;
        Mes mess_send;
        string ack;
        bool committed = false;
        bool is_aborted =false;
}P;
struct Client{
        string client_msg;
        string decision;
        int id;
        Mes mess_recv;
        string val;
        string value;
} Q[100];
bool mutex1 = false;

bool mutex2 = false;

bool mutex3 = false;

bool mutex4 = false;

void* Thread(void * par)
{
        int id_q = *((int *)par);

        while (mutex1 == true);
        mutex1 = true;

        P.mess_send.c = id_q;
        P.mess_send.message = P.prop;
        Q[id_q].mess_recv = P.mess_send;
        mutex1 = false;

        Q[id_q].id = Q[id_q].mess_recv.c;;
        Q[id_q].val = Q[id_q].mess_recv.message;;
        if (rand()%2) //用随机数模拟是否确认
        {
        Q[id_q].client_msg = "Commit";
        }
        else
        {
        Q[id_q].client_msg = "Abort";
        };
        while (mutex2 == true);
        mutex2 = true;

        P.server_msg = Q[id_q].client_msg;
        if(P.server_msg == "Abort"){
        P.is_aborted = true;
        }
        mutex2 = false;

        if(P.is_aborted ==false)
        {
        P.reply = "Commit";
        P.committed = true;
        }
        else{
        P.reply = "Abort";
        }
        while (mutex3 == true);
        mutex3 = true;

        Q[id_q].decision = P.reply;
        mutex3 = false;

        if(Q[id_q].decision=="Commit"){
        Q[id_q].value = Q[id_q].val;
        }
        while (mutex4 == true);
        mutex4 = true;

        P.ack = "Ack";
        mutex4 = false;

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