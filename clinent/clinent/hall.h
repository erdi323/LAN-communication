#ifndef HALL_H
#define HALL_H


void data_send(char* data, int sockfd); //发送信息
void data_recv(char* buffer, int sockfd);//接收消息
void func(int sockfd);//线程函数
void inputStr(char *buf, int len);//输入内容
void group_face(char *name,int sockfd);//群聊函数

void hall(char* name, int socked);
#endif // HALL_H
