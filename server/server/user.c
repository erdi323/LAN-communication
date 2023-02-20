#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

UN_t* tail = NULL;//尾节点

UN_t* find_by_name(UN_t* head, char* name)//查询用户
{
    UN_t* p = head->next;
    while (p != NULL)
    {
        if(strcmp(p->data.name, name) == 0)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

UN_t* find_by_sock(UN_t* head, int sockfd)//查询用户
{
    UN_t* p = head->next;
    while (p != NULL)
    {
        if(p->data.sockfd == sockfd)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

UN_t* create_node(Ud_t* data)//创建节点
{
    UN_t* new = calloc(1, sizeof (UN_t));
    if(new == NULL)
    {
        perror("calloc error:");
        exit(1);
    }
    if(data != NULL)
    {
        new->data = *data;
    }
    return new;
}

void insert_node(Ud_t* data)//尾插法
{
    UN_t *new = create_node(data);
    tail->next = new;
    new->prev = tail;
    tail = new;
}

void online_node(UN_t* head, char* name, pthread_t pid, int sockfd)//上线操作
{
    UN_t* p = find_by_name(head, name);
    if(head->next != p)
    {
        p->prev->next = p->next;
        if(p->next != NULL)
        {
            p->next->prev = p->prev;
        }
        else
        {
            tail = p->prev;
        }
        p->next = head->next;
        p->next->prev = p;
        p->prev = head;
        head->next =p;
    }
    p->data.pid = pid;
    p->data.sockfd = sockfd;
}
void unline_node(UN_t* head, char* name)//下线操作
{
    UN_t* p = find_by_name(head, name);
    if(p->next != NULL)
    {
        p->prev->next = p->next;
        p->next->prev = p->prev;
        tail->next = p;
        p->prev = tail;
        p->next = NULL;
        tail = p;
    }
    p->data.pid = -1;
    p->data.sockfd = -1;
}

#define PATH "data.bin"

void load_user()//读取用户信息
{
    FILE* fp = fopen(PATH,"rb");//二进制读取
    if(fp == NULL)
    {
        return;
    }
    Ud_t data = {{}, {}, -1, -1};
    while(fread(&(data.name), 21, 1, fp) > 0)//读取
    {
        fread(&(data.password), 21, 1, fp);
        insert_node(&data);
    }
    fclose(fp);
}

void save_user(UN_t* head)//储存用户信息
{
    FILE* fp = fopen(PATH,"wb");//二进制写入
    UN_t* p = head->next;
    while(p != NULL)
    {
        fwrite(&(p->data.name), 21, 1, fp);
        fwrite(&(p->data.password), 21, 1, fp);
        p = p->next;
    }
    fclose(fp);
}

void freeall(UN_t* head)//删除所有用户
{
    UN_t* p = head;
    UN_t* q = NULL;
    while(p != NULL)
    {
        q = p;
        p = p->next;
        free(q);
    }
}
