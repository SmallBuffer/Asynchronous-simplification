#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>

using namespace std;
const int client_num = 100;
vector<int> wait_queue;

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
    while (wait_queue.size()<client_num) {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock =
        accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    wait_queue.push_back(clnt_sock);
  }
}
int main() {
  int serv_sock=Init();
  listen(serv_sock, 20);//wait for clients to connect
  Connect_Clients(serv_sock);
  for (int i = 0; i < wait_queue.size();++i) //communicate with clients
  {
    int recv;
    read(wait_queue[i], &recv, sizeof(int));
    write(wait_queue[i], "ack", sizeof("ack"));
  }
  for(auto clnt_sock:wait_queue)//close socket
  close(clnt_sock);
  close(serv_sock);

  return 0;
}