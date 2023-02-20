#include "hall.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "Protocol.h"
#include "getch.h"

void data_send(char* data, int sockfd)
{
    int len = strlen(data) + 1;
    send(sockfd, &len, sizeof (int), 0);//发送内容长度
    send(sockfd, data, len, 0);//发送内容
}

void data_recv(char* buffer, int sockfd)
{
    int len = 0;
    recv(sockfd, &len, sizeof (int), 0);//接受内容长度
    recv(sockfd, buffer, len, 0);//接收内容
    printf("recv:%dbytes:%s\n",len, buffer);
}

void inputStr(char *buf, int len)
{
    char ch = 0;
    int i = 0;
    while(1)
    {
        ch = getch();
        if(ch == 127 || ch == '\b')
        {
            if(i > 0)
            {
                printf("\b \b");//退格
                buf[i] = '\0';
                i--;
            }
        }
        else if(i >= len)
        {
            printf("\a");//报警
        }
        else if(ch == '\n')
        {
            printf("\n");
            buf[i] = '\0';
            break;
        }
        else
        {
            printf("%c",ch);
            buf[i] = ch;
            i++;
        }
    }

}

char G_buf[BUFSIZ];//群聊缓存
char P_buf[BUFSIZ];//私聊缓冲

char user_list[20][21];//用户数组
int u_len;//在线用户数
char current[21];//私聊选择的用户

int modeF = 0;//用户所在模式标志 0大厅，1群聊，2私聊选择，3私聊
int exit_f = 0;//退出标志

//在线用户刷新
void fflush_logon_user(int sockfd)
{
    int ch = 1;
    u_len = 0;
    memset(user_list, 0 ,sizeof (user_list));//清空
    while (u_len < 20)
    {
        recv(sockfd, &ch, sizeof (int), 0);
        if(ch == 0)
        {
            break;
        }
        data_recv(user_list[u_len],sockfd);
        u_len++;
    }
}

//从文件中读取私聊信息
void msg_load(char* path)
{
    system("clear");
    FILE *fp = fopen(path,"r");
    if(fp == NULL)
    {
        return;
    }
    char buf[BUFSIZ+1];
    while(fgets(buf, BUFSIZ, fp) != NULL)
    {
        printf("%s\n", buf);
    }
    fclose(fp);
}

//私聊信息保存到文件中
void msg_save(int sockfd)
{
    char buf[BUFSIZ+1];
    char path[BUFSIZ+1] = "./";//文件路径
    data_recv(buf, sockfd); //接受自己的名字
    strcpy(path, buf);
    if(access(path, 0) < 0)//判断是否存在
    {
        char cmd[BUFSIZ+1] = "mkdir ./";
        strcat(cmd,buf);
        system(cmd);
    }
    strcat(path, "/");
    memset(buf, 0, sizeof(buf));

    char other[21];
    data_recv(other, sockfd); //接受对方的名字
    strcat(path, other);
    data_recv(buf, sockfd); //接受消息
    strcat(buf, "\n");

    FILE*fp = fopen(path, "a");
    if(fp == NULL)
    {
        printf("%s\n", path);
        return;
    }
    fputs(buf, fp);
    fclose(fp);
    if(modeF == 3 && strcmp(other,current)==0)
    {
        msg_load(path);
    }
}


//私聊
void private_chat(char* me, char* other, int sockfd)
{
    char buf[BUFSIZ+1];
    strcpy(buf, me);
    strcat(buf, "/");
    strcat(buf, other);
    msg_load(buf);
    memset(buf, 0, sizeof (buf));//清空
    strcpy(buf,me);
    strcat(buf,":");
    printf("===============\n");
    while(1)
    {
        inputStr(P_buf, BUFSIZ);
        if(strcmp(P_buf, "end") == 0)
        {
            break;
        }
        strcat(buf, P_buf);
        strcat(buf, "\n");
        int ch = REQUEST_PRIVATEMSG;
        send(sockfd, &ch, sizeof(int), 0);
        data_send(other, sockfd);
        data_send(me, sockfd);
        data_send(buf, sockfd);
        memset(P_buf, 0, sizeof (P_buf));
    }
}

//私聊选择
void private_face(char *name, int sockfd)
{

    while (1)
    {
        system("clear");
        for(int i = 0; i< u_len; i++)//显示当请再线用户
        {
            printf("%d.%s ",i+1, user_list[i]);
        }
        printf("\n0.退出\n");
        printf("请选择私聊对象：");
        int ch = -1;
        scanf("%d",&ch);
        if(ch == 0)
        {
            return;
        }
        else
        {
            modeF = 3;
            memset(current, 0, sizeof(current));
            strcpy(current, user_list[ch-1]);
            private_chat(name, user_list[ch-1], sockfd);
            modeF = 2;
        }
    }

}

//群聊
void group_face(char *name,int sockfd)
{
    system("clear");
    printf("-------------------------------------\n");
    while(1)
    {
        inputStr(G_buf, BUFSIZ);//输入
        if(strcmp(G_buf, "end") == 0)
        {
            break;
        }
        int ch = REQUEST_GROUPMSG;//群聊请求
        send(sockfd, &ch, sizeof (int), 0);//发送请求
        data_send(name, sockfd);//发送名字
        data_send(G_buf, sockfd);//发送内容
        memset(G_buf, 0, sizeof (G_buf));//清空
    }
}

//用线程接受消息
void func(int sockfd)
{
    printf("\nrecv on....\n");
    char group_buf[BUFSIZ*4];
    while (1)
    {
        int type = 0;
        recv(sockfd, &type, sizeof(int), 0);
        printf("type: %d\n",type);
        char buffer[BUFSIZ+1];
        switch (type)
        {
        case REPLAY_GROUPMSG://群聊回复 8
            data_recv(buffer, sockfd);
            strcat(group_buf, buffer);
            strcat(group_buf, "\n");
            if(modeF == 1)
            {
                system("clear");
                printf("%s", group_buf);
                printf("--------------------------\n%s", G_buf);
                fflush(stdout);
            }
            break;
        case REPLAY_PRIVATEMSG://私聊回复 9
            msg_save(sockfd);
            break;
        case REPLAY_FFLUSH://刷新用户 10
            fflush_logon_user(sockfd);
            if(modeF == 2)
            {
                system("clear");
                for(int i = 0; i< u_len; i++)
                {
                    printf("%d.%s ",i+1, user_list[i]);
                }
                printf("\n0.退出\n");
                fflush(stdout);
            }
            break;
        case REPLAY_EXIT:
            exit_f = 1;
            return;
        default:
            printf("error\n");
            break;
        }
    }
}

void hall(char* name, int sockfd)
{
    int ch = -1;
    exit_f = 0;
    pthread_t pid;
    pthread_create(&pid, NULL, (void*)func, (void*)(intptr_t)sockfd);
    while(1)
    {
        printf("1.群聊\n");
        printf("2.私聊\n");
        printf("0.退出\n");
        printf("请输入选择:");
        scanf("%d",&ch);
        while (getchar() != '\n');
        switch (ch)
        {
        case 1:
            modeF = 1;
            group_face(name, sockfd);
            modeF = 0;
            break;
        case 2:
            modeF = 2;
            private_face(name, sockfd);
            modeF = 0;
            break;
        case 0:
            ch = REQUEST_EXIT;//下线请求
            send(sockfd, &ch, sizeof(int), 0);
            data_send(name, sockfd);
            return;
        default:
            break;
        }
    }
}
