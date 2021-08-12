#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include<assert.h>
#include <string>
using namespace std;

const int client_num = 100;
vector<int> wait_queue;
string bufRecv;

int Init()
{
    //创建套接字
  int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  //将套接字和IP、端口绑定
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr)); //每个字节都用0填充
  serv_addr.sin_family = AF_INET;           //使用IPv4地址
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //具体的IP地址
  serv_addr.sin_port = htons(1234);                   //端口
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

  for (int i = 0; i < wait_queue.size();++i)
  {
      write(wait_queue[i], "ping", sizeof("ping"));
  }
  for (int i = 0; i < wait_queue.size();++i)
  {
      read(wait_queue[i], &bufRecv, sizeof(string));
      assert(bufRecv == "pong");
  }

  //关闭套接字
  for(auto clnt_sock:wait_queue)
  close(clnt_sock);
  close(serv_sock);

  return 0;
}
