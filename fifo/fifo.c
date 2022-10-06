#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "fifo.h"

void CheckFifoExistence()
{
    int res;

    if (access(FIFO0_PATH, F_OK) == -1)
    {
        res = mkfifo(FIFO0_PATH, 0777);
        if (res != 0)
        {
            fprintf(stderr, "FIFO0 creation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    if (access(FIFO1_PATH, F_OK) == -1)
    {
        res = mkfifo(FIFO1_PATH, 0777);
        if (res != 0)
        {
            fprintf(stderr, "FIFO1 creation failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

void ReadAckMessage(char *pipe, struct Message *message)
{
    int pid = getpid();

    if (message->ack == 0)
    {
        int pipe_fd = open(pipe, O_RDONLY);

        if (pipe_fd != -1)
        {
            char ack = 1;
            int res = read(pipe_fd, (void *)&ack, 1);

            if (res == -1)
            {
                fprintf(stderr, "%s read error in process %d\n", pipe, pid);
                free(message);
                exit(EXIT_FAILURE);
            }
            else
            {
                if (ack == 1)
                {
                    printf("Process %d --- the message sent is successfully received.\n", pid);
                }
                else
                {
                    printf("Process %d --- the message sent is not successfully received.\n", pid);
                }

                message->ack = 1;
            }
        }
    }
}

void WriteAckMessage(char *ackPipe, struct Message* message)
{
    int pipe_fd = open(ackPipe, O_WRONLY);
    
    if(pipe_fd!=-1)
    {
        char ack = 1;
        int res = write(pipe_fd, (void *)&ack, 1);

        if(res==-1)
        {
            int pid = getpid();
            fprintf(stderr, "%s write error in process %d\n", pipe, pid);
            free(message);
            exit(EXIT_FAILURE);
        }

        close(pipe_fd);
    }
}

void ReadMessage(char *pipe, char *ackPipe, struct Message *message)
{
    ReadAckMessage(pipe, message);

    int pipe_fd = open(pipe, O_RDONLY);
    int pid = getpid();

    if (pipe_fd != -1)
    {
        int res = read(pipe_fd, (void *)message, PIPE_BUF);

        if (res == -1)
        {
            fprintf(stderr, "%s read error in process %d\n", pipe, pid);
            free(message);
            exit(EXIT_FAILURE);
        }

        if (res > 0)
        {
            printf("Process %d --- successfully read message from process %d. The message content is \"%s\"\n", pid, message->pid, message->buf);
            close(pipe_fd);

            if (strcmp(message->buf, "END") == 0)
            {
                free(message);
                exit(EXIT_SUCCESS);
            }

            WriteAckMessage(ackPipe, message);
        }
    }
}

void WriteMessage(char *pipe, struct Message *message)
{
    int pid = getpid();
    printf("Process %d --- input the message sent to %s:", pid, pipe);
    gets(message->buf);

    int pipe_fd = open(pipe, O_WRONLY);
    message->pid = pid;

    if (pipe_fd != -1)
    {
        message->ack = 0;
        int res = write(pipe_fd, (void *)message, PIPE_BUF);

        if(strcmp(message->buf,"END")==0)
        {
            free(message);
            exit(EXIT_SUCCESS);
        }

        if (res == -1)
        {
            fprintf(stderr, "%s write error in process %d\n", pipe, pid);
            free(message);
            exit(EXIT_FAILURE);
        }

        close(pipe_fd);
    }
}
