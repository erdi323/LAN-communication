#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Protocol.h"
#include "hall.h"

int clinent_start()
{
    //创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);//IPV4,tcp
    if(sockfd < 0)
    {
        perror("socket error");
        exit(1);
    }

    //设置ip
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//设置通信协议 ipv4
    addr.sin_port = htons(8888);//设置端口
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");//设置套接字的地址 htonl(INADDR_ANY);
    //在终端使用ifconfig查看
    //本机网关（inet 地址:192.168.1.47）
    //本地网关（inet 地址:127.0.0.1）

    //绑定ip
    if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
    {
        perror("connect error:");
        exit(1);
    }
    return sockfd;
}

void regist_login(int sockfd)
{
    char name[21];
    char password[21];
    printf("请输入名字:");
    scanf("%s",name);
    while(getchar() != '\n');
    printf("请输入密码:");
    scanf("%s",password);
    while(getchar() != '\n');
    data_send(name, sockfd);//发送名字
    data_send(password, sockfd);//发送密码

    char buffer[BUFSIZ+1];
    data_recv(buffer,sockfd);
    printf("%s\n",buffer);
    if(strcmp(buffer,LOGIN_OK) == 0)
    {
        hall(name, sockfd);//进入大厅
    }
}

int main()
{
    int sockfd = clinent_start();
    int ch= -1;
    while(1)
    {
        system("clear");
        printf("1.注册\n");
        printf("2.登录\n");
        printf("0.退出\n");
        printf("请选择：");
        scanf("%d",&ch);
        switch (ch)
        {
        case 1:
            ch = REQUEST_REGIST;//注册请求
            send(sockfd, &ch, sizeof (int), 0);
            regist_login(sockfd);
            break;
        case 2:
            ch = REQUEST_LOGIN;//登录请求
            send(sockfd, &ch, sizeof (int), 0);
            regist_login(sockfd);
            break;
        default:
            close(sockfd);
            return 0;
        }
    }
    return 0;
}
