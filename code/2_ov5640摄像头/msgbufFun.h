#ifndef _MSGBUF_FUN_H_
#define _MSGBUF_FUN_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>


struct msgbuf
{
    long mtype;
    int num;
};


int createMsgQueue();
int sendMsgQueue(int msg_id, int who, int msgdata);
int recvMsgQueue(int msg_id, int recvType);
#endif
