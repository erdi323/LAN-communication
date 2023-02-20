#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "user.h"
#include "Protocol.h"

UN_t* head = NULL;
extern UN_t* tail;

//服务器开始处理，包括套接字创建，绑定ip，返回套接字
int server_start()
{
    //创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);//IPV4,tcp
    if(sockfd < 0)
    {
        perror("sockfd error");
        exit(1);
    }

    //设置ip
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//设置通信协议 ipv4
    addr.sin_port = htons(8888);//设置端口
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//设置套接字的地址 inet_addr("127.0.0.1")

    //绑定ip
    if(bind(sockfd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
    {
        perror("bind error:");
        exit(1);
    }

    //监听
    if(listen(sockfd, 20) < 0)
    {
        perror("listen error:");
        exit(1);
    }
    return sockfd;
}

void data_send(char* data, int sockfd)
{
    int len = strlen(data) + 1;
    send(sockfd, &len, sizeof (int), 0);//发送内容长度
    send(sockfd, data, len, 0);//发送内容
}

void data_recv(char* buffer, int sockfd)
{
    int len;
    recv(sockfd, &len, sizeof (int), 0);//接受内容长度
    recv(sockfd, buffer, len, 0);//接收内容
    printf("recv:%dbytes:%s\n",len, buffer);
}

void regist(int sockfd)
{
    Ud_t temp = {{}, {}, -1, -1};
    data_recv(temp.name, sockfd);
    data_recv(temp.password, sockfd);
    if(find_by_name(head,temp.name) == NULL)
    {
        insert_node(&temp);
        data_send(REGIST_OK,sockfd);
        save_user(head);
    }
    else
    {
        data_send(REGIST_FAIL, sockfd);
    }

}

//在线用户刷新
void fflush_logon_user()
{
    log;
    UN_t* send_uesr = head->next;//要发送的用户
    while (send_uesr != NULL)
    {
        if(send_uesr->data.sockfd < 0)
        {
            log;
            break;
        }
        printf("sendto %s\n", send_uesr->data.name);
        int ch = REPLAY_FFLUSH;//刷新用户
        send(send_uesr->data.sockfd, &ch, sizeof (int), 0);

        //q为取在线用户的名字
        for(UN_t *login_user = head->next; login_user != NULL; login_user = login_user->next)
        {
            if(login_user->data.sockfd < 0)
            {
                log;
                break;
            }
            ch = 1;
            if(strcmp(send_uesr->data.name, login_user->data.name) != 0)
            {
                send(send_uesr->data.sockfd, &ch, sizeof (int), 0);
                printf("  user:%s", login_user->data.name);
                data_send(login_user->data.name, send_uesr->data.sockfd);
            }
        }
        ch = 0;
        send(send_uesr->data.sockfd, &ch, sizeof (int),0);
        printf("\n");
        send_uesr = send_uesr->next;
    }
}


void login(int sockfd)
{
    Ud_t temp = {{}, {}, -1, -1};
    data_recv(temp.name, sockfd);
    data_recv(temp.password, sockfd);

    UN_t *p = find_by_name(head, temp.name);
    if(p == NULL)
    {
        data_send(LOGIN_NAME_FAIL, sockfd);
    }
    else if(strcmp(p->data.password, temp.password) != 0)
    {
        data_send(LOGIN_PASSWORD_FAIL, sockfd);
    }
    else if(p->data.sockfd != -1)
    {
        data_send(LOGIN_FAIL, sockfd);
    }
    else
    {
        data_send(LOGIN_OK, sockfd);
        //获取当前线程号:pthread_self()
        online_node(head, p->data.name, pthread_self(), sockfd);
        fflush_logon_user();
    }
}

void user_exit(int sockfd)//下线
{
    char name[21];
    data_recv(name, sockfd);
    unline_node(head, name);
    fflush_logon_user();
}

void groupMsg(int sockfd)//群聊
{
    char buffer1[BUFSIZ+1];
    char buffer2[BUFSIZ+1];
    data_recv(buffer1, sockfd);
    data_recv(buffer2, sockfd);
    strcat(buffer1, ":");//拼接
    strcat(buffer1, buffer2);
    log;
    printf("%s\n",buffer1);
    UN_t *p = head->next;
    while(p != NULL)
    {
        int ch = REPLAY_GROUPMSG;//群聊回复 8
        send(p->data.sockfd, &ch, sizeof(int), 0);
        data_send(buffer1, p->data.sockfd);//转发
        p = p->next;
    }
}

void PrivateMsg(int sockfd)//私聊
{
    char recv_name[21];//接受者
    char send_name[21];//发送者
    char msg[BUFSIZ+1];//消息
    int ch = REPLAY_PRIVATEMSG;//私聊回复
    data_recv(recv_name, sockfd);
    data_recv(send_name, sockfd);
    data_recv(msg, sockfd);

    //先给接受者发送消息
    UN_t* r = find_by_name(head, recv_name);
    if(r != NULL)
    {
        send(r->data.sockfd, &ch, sizeof (int), 0);
        data_send(recv_name, r->data.sockfd);
        data_send(send_name, r->data.sockfd);
        data_send(msg, r->data.sockfd);
    }
    //再给发送者反馈消息
    send(sockfd, &ch, sizeof (int), 0);
    data_send(recv_name, sockfd);
    data_send(send_name, sockfd);
    data_send(msg, sockfd);
}

//线程函数
void func(int sockfd)
{
    int ch = -1;
    while(1)
    {
        int n = recv(sockfd, &ch, sizeof (int), 0);
        if(n <= 0)
        {
            printf("客户端断开连接\n");
            UN_t* p = find_by_sock(head,sockfd);
            if(p!=NULL)
            {
                unline_node(head,p->data.name);
                fflush_logon_user();
            }
            break;
        }
        printf("%d\n",ch);
        switch (ch)
        {
        case REQUEST_REGIST://注册请求 0
            printf("新用户注册\n");
            regist(sockfd);
            break;
        case REQUEST_LOGIN://登录请求 1
            printf("用户登录\n");
            login(sockfd);
            break;
        case REQUEST_EXIT://下线请求 2
            printf("用户下线\n");
            user_exit(sockfd);
            break;
        case REQUEST_GROUPMSG://群聊请求 3
            printf("群聊请求\n");
            groupMsg(sockfd);
            break;
        case REQUEST_PRIVATEMSG://私聊请求 4
            printf("私聊请求\n");
            PrivateMsg(sockfd);
            break;
        default:
            break;
        }
    }
}

int main()
{
    head = create_node(NULL);
    tail = head;
    load_user();//读取用户
    pthread_t pid;
    int listenfd = server_start();
    while(1)
    {
        int sockfd = accept(listenfd, NULL, NULL);//接受连接
        if(sockfd < 0)
        {
            perror("accpet error");
            exit(1);
        }
        else
        {
            printf("新客户端已连接成功\n");
            pthread_create(&pid, NULL, (void*)func, (void*)(intptr_t)sockfd);
        }
    }
    return 0;
}
