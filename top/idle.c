#include <stdio.h>
#include <unistd.h>
#include <math.h>

void compute()
{
    while(1)
    {
        static double a = 1278.0;
        a = log(a);
    }
}

int main()
{
    fork();
    fork();
    compute();
    return 1;
}