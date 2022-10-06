#include "fifo.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct Message *message=malloc(sizeof(struct Message));
    message->ack = 1;
    CheckFifoExistence();

    while(1)
    {
        ReadMessage(FIFO0_PATH, FIFO1_PATH, message);
        WriteMessage(FIFO1_PATH, message);
    }

    return 0;
}