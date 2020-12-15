#pragma once

void abortWithMessage(const char *message)
{
    printf("Aborting: %s\n", message);
    abort();
}