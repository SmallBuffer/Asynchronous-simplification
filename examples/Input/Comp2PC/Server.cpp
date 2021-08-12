#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include<assert.h>
#include<stdbool.h>
#include<string>
using namespace std;

const int client_num = 100;
vector<int> wait_queue;
string server_msg;
string prop;
bool committed = false;
bool is_aborted =false;
struct Mes
{
    string message;
    int c;
};

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
  listen(serv_sock, 20);
  Connect_Clients(serv_sock);

  string reply;
  for (int i = 0; i < wait_queue.size(); ++i)
  {
      Mes mess_send;
      mess_send.c = i;
      mess_send.message=prop;
      write(wait_queue[i], (char *)&mess_send, sizeof(Mes));
  }
  for (int i = 0; i < wait_queue.size();++i)
  {
      read(wait_queue[i], &server_msg, sizeof(server_msg));
      if(server_msg == "Abort"){
          is_aborted = true;
      }
  }
  if(is_aborted ==false)
  {
      reply = "Commit";
      committed = true;
  }
  else{
      reply = "Abort";
  }
  for (int i = 0; i < wait_queue.size();++i)
  {
      write(wait_queue[i], &reply, sizeof(reply));
  }
  string ack;
  for (int i = 0; i < wait_queue.size(); ++i)
  {
      read(wait_queue[i], &ack, sizeof(ack));
  }

  for(auto clnt_sock:wait_queue)
  close(clnt_sock);
  close(serv_sock);

  return 0;
}