#ifndef PROTOCOL_H
#define PROTOCOL_H

#define log (printf("[%s:第%d行 %s] ", __FILE__, __LINE__, __FUNCTION__))

#define REGIST_OK "注册成功"
#define REGIST_FAIL "注册失败，用户名已存在"

#define LOGIN_OK "登录成功"
#define LOGIN_FAIL "帐号已在线"
#define LOGIN_PASSWORD_FAIL "登录失败，密码错误"
#define LOGIN_NAME_FAIL "登录失败，用户不存在"

enum
{
    REQUEST_REGIST = 0, //注册请求 0
    REQUEST_LOGIN,      //登录请求 1
    REQUEST_EXIT,       //下线请求 2
    REQUEST_GROUPMSG,   //群聊请求 3
    REQUEST_PRIVATEMSG, //私聊请求 4

    REPLAY_REGIST,      //注册回复 5
    REPLAY_LOGIN,       //登录回复 6
    REPLAY_EXIT,        //退出回复 7
    REPLAY_GROUPMSG,    //群聊回复 8
    REPLAY_PRIVATEMSG,  //私聊回复 9
    REPLAY_FFLUSH       //刷新用户 10
};

#endif // PROTOCOL_H
