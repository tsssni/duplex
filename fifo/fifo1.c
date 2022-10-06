#include "fifo.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct Message *message=malloc(sizeof(struct Message));
    message->ack = 1;
    CheckFifoExistence();

    while(1)
    {
        WriteMessage(FIFO0_PATH, message);
        ReadMessage(FIFO1_PATH, FIFO0_PATH, message);
    }

    return 0;
}