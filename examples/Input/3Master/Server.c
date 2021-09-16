#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

const int client_num = 100;
int wait_queue[102];

int Init()
{
  int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr)); 
  serv_addr.sin_family = AF_INET;           
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
  serv_addr.sin_port = htons(1234);                   
  bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  return serv_sock;
}
void Connect_Clients(int serv_sock)
{
  int cnt = 0;
  while (cnt < client_num)
  {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock =
        accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    wait_queue[cnt++] = clnt_sock;
  }
}
int main() 
{
  int serv_sock=Init();
  listen(serv_sock, 20);//wait for clients to connect
  Connect_Clients(serv_sock);
  for (int i = 0; i < client_num;++i) //communicate with clients
  {
    int send,recv;
    read(wait_queue[i], &recv, sizeof(int));
    send = 2;
    write(wait_queue[i], &send, sizeof(int));
  }
  for (int i = 0; i < client_num; ++i)//close socket
  close(wait_queue[i]);
  close(serv_sock);

  return 0;
}

/*
1 => pong
2 => ping
3 => ack
*/
