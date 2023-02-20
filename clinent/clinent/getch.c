#include "getch.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>//终端头文件
#include <string.h>

char getch(void)
{
    char ch = 0;
    //修改终端设置 - 无回车输出，无回显
    struct termios tm, tmold;
    tcgetattr(STDIN_FILENO,&tm);
    //tcgetattr函数检索与文件描述符关联的终端设置
    //文件描述符STDIN_FILENO是一个扩展为整数值 0 并表示标准输入流的宏
    tmold = tm;
    tm.c_lflag &= ~(ICANON | ECHO);//ICANON关闭终端缓冲区 ECHO
    tcgetattr(STDIN_FILENO,&tm);
    ch = getchar();
    tcgetattr(STDIN_FILENO,&tmold);
    return ch;
}
