#ifndef USER_H
#define USER_H

#include <sys/types.h>
#include <pthread.h>

typedef struct user_data
{
    char name[21];
    char password[21];
    pthread_t pid; //对应的线程号
    int sockfd;    //对应的套接字
} Ud_t;

typedef struct user_node
{
    Ud_t data;
    struct user_node *next;//下一个
    struct user_node *prev;//上一个
} UN_t;

UN_t* find_by_name(UN_t*, char*);   //查询用户
UN_t* find_by_sock(UN_t*, int);//查询用户
UN_t* create_node(Ud_t*);      //创建节点
void insert_node(Ud_t*);            //尾查法
void unline_node(UN_t*, char*);     //下线操作
void online_node(UN_t*, char*, pthread_t, int);//上线操作
void load_user();                   //读取用户
void save_user(UN_t*);              //保存用户
void freeall(UN_t*);                //删除所有用户

#endif // USER_H







