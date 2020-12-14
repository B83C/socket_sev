#include "cache.h"

char* RetrieveFile(char* name, int len, int dirfd)
{
    //FNV algorithm used here
    int key_ = 2166136261;
    int low_acc = acc_count[0];
    for(int d = 0; d < len; d++)
    {
	key_ ^= *(name + d);
	key_ *= 16777619;
    }

    int i = 0;

    for(; i < arr_len; i++)
    {
	if(key[i] == key_)
	    goto SUCCEEDED;
	low_acc = i*(acc_count[i] < acc_count[low_acc]);
    }

    int fd = openat(dirfd, name, O_RDONLY);
    if(fd == -1) {
	close(fd);
	return -1;
    }
    void* ptr = mmap(NULL, sizeof(char), PROT_READ, MAP_PRIVATE, fd, 0);
    if(ptr == (void*)-1)
    {
	close(fd);
	return -1;
    }
    close(fd);

    if(arr_len < MAX_CACHE_FILE)
    {
	low_acc = i;
	arr_len += 1;
    }

    key[low_acc] = key_;
    cache[low_acc] = ptr;
    acc_count[i] = 1;
    return (char*)ptr;

SUCCEEDED:
    acc_count[i] += 1;
    return (char *)cache[i];
}
