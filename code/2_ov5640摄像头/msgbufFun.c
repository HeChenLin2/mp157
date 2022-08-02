#include "msgbufFun.h"


static int commMsgQueue(int flags)
{
    key_t key = ftok("/tmp", 0x6666);
    if(key < 0)
    {
        perror("ftok");
        return -1;
    }

    int msg_id = msgget(key, flags);
    if(msg_id < 0)
    {
        perror("msgget");
    }
    return msg_id;
}


int createMsgQueue()
{
    return commMsgQueue(IPC_CREAT|0666);
}


int sendMsgQueue(int msg_id, int who, int msgdata)
{
    struct msgbuf buf;
    buf.mtype = who;
   	buf.num = msgdata;

    if(msgsnd(msg_id, (void*)&buf, sizeof(buf.num), 0) < 0)
    {
        perror("msgsnd");
        return -1;
    }
    return 0;
}

int recvMsgQueue(int msg_id, int recvType)
{
    struct msgbuf buf;
    int size=sizeof(buf.num);
    if(msgrcv(msg_id, (void*)&buf, size, recvType, 0) < 0)
    {
        perror("msgrcv");
        return -1;
    }
    return buf.num;
}

