#include "wrapper.h"

int GetError()
{
    #ifdef _WIN32
    return WSAGetLastError();
    #endif
    return errno;
}
