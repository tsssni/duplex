#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>

sem_t mutex;
int pid;

void InitServerSocket(int *sockfd, struct sockaddr_in *sockAddr, fd_set *readfds, int port)
{
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockAddr->sin_family = AF_INET;
    sockAddr->sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr->sin_port = htons(port);

    int sockLen = sizeof(*sockAddr);
    if (bind(*sockfd, (struct sockaddr *)sockAddr, sockLen) == 0)
    {
        printf("Server process %d --- succeeded to bind.\n", pid);
    }
    else
    {
        printf("Server process %d --- failed to bind.\n", pid);
        exit(EXIT_FAILURE);
    }

    listen(*sockfd, 5);
    FD_ZERO(readfds);
    FD_SET(*sockfd, readfds);
}

void *CountMessageNumPerSec(void *messageCountArg)
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
            printf("Server process %d --- receive %d messages last second\n", pid, *messageCount);
            sem_wait(&mutex);
            *messageCount = 0;
            sem_post(&mutex);

            lastTime = currTime;
        }
    }
}

void HandleServerConnection(int *serverSockfd, fd_set *readfds)
{
    int clientSockfd;
    struct sockaddr_in clientAddr;
    int clientLen;

    char buf[PIPE_BUF];
    int readLen;

    int fd;
    fd_set testfds;

    int maxfd = *serverSockfd;
    int currMax = maxfd;

    int messageCount;
    int selectRes;
    int nRead;

    struct timeval waitTime;
    waitTime.tv_sec = 30;

    sem_init(&mutex, 0, 1);
    pthread_t messagePerSecThread;
    pthread_create(&messagePerSecThread, NULL, CountMessageNumPerSec, (void *)&messageCount);

    while (1)
    {
        testfds = *readfds;
        currMax = maxfd;

        selectRes = select(FD_SETSIZE, &testfds, NULL, NULL, &waitTime);
        waitTime.tv_sec = 30;

        if (selectRes == -1)
        {
            printf("Server process %d --- failed to select\n", pid);
            sem_destroy(&mutex);
            exit(EXIT_FAILURE);
        }
        else if (selectRes == 0)
        {
            printf("Server process %d --- exceeded the maximal wait time, the server process will be terminated.\n", pid);
            sem_destroy(&mutex);
            exit(EXIT_SUCCESS);
        }

        for (fd = 0; fd <= maxfd; ++fd)
        {
            if (FD_ISSET(fd, &testfds))
            {
                if (fd == *serverSockfd)
                {
                    clientLen = sizeof(clientAddr);
                    clientSockfd = accept(*serverSockfd, (struct sockaddr *)&clientAddr, &clientLen);
                    FD_SET(clientSockfd, readfds);

                    if (clientSockfd > maxfd)
                    {
                        maxfd = clientSockfd;
                    }

                    printf("Server process %d --- added client on fd %d\n", pid, clientSockfd);
                }
                else
                {

                    readLen = read(fd, buf, PIPE_BUF);

                    if (readLen != -1)
                    {
                        sem_wait(&mutex);
                        ++messageCount;
                        sem_post(&mutex);

                        if (strcmp(buf, "EXIT") == 0)
                        {
                            close(fd);
                            FD_CLR(fd, readfds);
                            printf("Server process %d --- remove client on fd %d\n", pid, fd);
                        }
                        else
                        {
                            write(fd, "1", 1);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int serverSockfd;
    struct sockaddr_in serverAddr;
    fd_set readfds;

    pid = getpid();
    int port;

    if (argc > 2)
    {
        printf("Server process %d --- type a number argument to set the port or type none to use the default.\n", pid);
        exit(EXIT_FAILURE);
    }
    else if (argc == 2)
    {
        port = atoi(argv[1]);
        if (port < 1024)
        {
            printf("Server process %d --- type a number larger than 1024.\n", pid);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        port = 9734;
    }

    InitServerSocket(&serverSockfd, &serverAddr, &readfds, port);
    HandleServerConnection(&serverSockfd, &readfds);

    return 0;
}