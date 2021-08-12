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
#include<string>
using namespace std;

const int client_num = 20;
const int round = 100;
vector<int> wait_queue;
struct Mes{
    string message;
    int round;
};
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
  //进入监听状态，等待用户发起请求
  listen(serv_sock, 20);
  Connect_Clients(serv_sock);

  //向客户端发送数据
  for(int r=0;r<round;++r)
  {
    for (int i = 0; i < wait_queue.size();++i)
    {
        Mes server_send,server_recv;
        server_send.message="ping";
        server_send.round = r;
        write(wait_queue[i], &server_send, sizeof(Mes));
        read(wait_queue[i], &server_recv, sizeof(Mes));
        assert(server_recv.message == "pong");
    }
  }
 

  //关闭套接字
  for(auto clnt_sock:wait_queue)
  close(clnt_sock);
  close(serv_sock);

  return 0;
}