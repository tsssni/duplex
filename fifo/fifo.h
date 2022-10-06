#ifndef FIFO_H
#define FIFO_H

#include <fcntl.h>
#include <limits.h>
#define FIFO0_PATH "/tmp/fifo0"
#define FIFO1_PATH "/tmp/fifo1"

struct Message
{
    int pid;
    int ack;
    char buf[PIPE_BUF];
};

void CheckFifoExistence();
void ReadMessage(char *pipe, char *ackPipe, struct Message *message);
void WriteMessage(char *pipe, struct Message *message);

#endif