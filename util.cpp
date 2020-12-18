#pragma once

#include <stdio.h>
#include <stdlib.h>

[[noreturn]]
void abortWithMessage(const char *message)
{
    printf("Aborting: %s\n", message);
    abort();
}

void assertOrAbort(bool condition, const char *message)
{
    if (!condition)
    {
        abortWithMessage(message);
    }
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

double getRandomDouble()
{
    return 1.0 * rand() / RAND_MAX;
}