#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

sem_t mutex;
int pid;

void InitClientSocket(int *sockfd, struct sockaddr_in *sockAddr, int port)
{
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int pid = getpid();

    sockAddr->sin_family = AF_INET;
    sockAddr->sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr->sin_port = htons(port);

    int sockLen = sizeof(*sockAddr);
    if (connect(*sockfd, (struct sockaddr *)sockAddr, sockLen) == -1)
    {
        printf("Client process %d --- connection failed.\n", pid);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Client process %d ---  connection to port %d succeeded\n", pid, port);
    }
}

void* CountMessageNumPerSec(void* messageCountArg)
{
    struct timespec lastTime;
    struct timespec currTime;
    int *messageCount = (int *)messageCountArg;

    clock_gettime(CLOCK_REALTIME, &currTime);
    lastTime = currTime;
    
    while (1)
    {
        clock_gettime(CLOCK_REALTIME, &currTime);
        if (currTime.tv_sec - lastTime.tv_sec >= 1)
        {
            printf("Server process %d --- sent %d messages last second\n", pid, *messageCount);

            sem_wait(&mutex);
            *messageCount = 0;
            sem_post(&mutex);

            lastTime = currTime;
        }
    }
}

void HandleClientConnection(int *clientSocketfd, int mesNum)
{
    char buf[PIPE_BUF];
    char serverMessage;
    pid = getpid();

    int messageCountPerSec = 0;
    struct timespec startTime;
    struct timespec currTime;
    struct timespec lastTime;

    sem_init(&mutex, 0, 1);
    pthread_t messageCountThread;
    pthread_create(&messageCountThread, NULL, CountMessageNumPerSec, (void *)&messageCountPerSec);

    clock_gettime(CLOCK_REALTIME, &currTime);
    startTime = currTime;
    lastTime = currTime;

    while (currTime.tv_sec - startTime.tv_sec < 30)
    {
        clock_gettime(CLOCK_REALTIME, &currTime);

        if(messageCountPerSec >= mesNum)
        {
            continue;
        }

        sprintf(buf, "%d", pid);
        write(*clientSocketfd, buf, PIPE_BUF);
        read(*clientSocketfd, &serverMessage, 1);

        if (serverMessage == '1')
        {
            sem_wait(&mutex);
            ++messageCountPerSec;
            sem_post(&mutex);
        }
        else
        {
            printf("Client process %d --- failed to send message.\n", pid);
            exit(EXIT_FAILURE);
        }
    }

    write(*clientSocketfd, "EXIT", PIPE_BUF);
}

int main(int argc, char *argv[])
{
    int clientSocketfd;
    struct sockaddr_in clientSockAddr;

    int pid = getpid();
    int port;
    int mesNum = INT_MAX;

    if (argc > 3)
    {
        printf("Server process %d --- type a number argument to set the port or type none to use the default.\n", pid);
        exit(EXIT_FAILURE);
    }
    else if (argc == 2 || argc == 3)
    {
        port = atoi(argv[1]);
        if (port < 1024)
        {
            printf("Server process %d --- type a number larger than 1024.\n", pid);
            exit(EXIT_FAILURE);
        }

        if(argc==3)
        {
            mesNum = atoi(argv[2]);
        }
    }
    else
    {
        port = 9734;
    }

    InitClientSocket(&clientSocketfd, &clientSockAddr, port);
    HandleClientConnection(&clientSocketfd, mesNum);

    return 0;
}