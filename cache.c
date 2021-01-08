#include "cache.h"

char* RetrieveFile(char* name, int len, int dirfd)
{
    int low_acc = acc_count[0];
    XXH64_hash_t key_ = XXH64(name, len, 0);

    int i = 0;

    for(; i < arr_len; i++)
    {
	if(key[i] == key_)
	    goto SUCCEEDED;
	low_acc = i*(acc_count[i] < acc_count[low_acc]);
    }

    int fd = openat(dirfd, name, O_RDONLY);
    if(fd == -1) {
//	fprintf(stderr, "Unable to retrieve file : %s, filename : %s\n", strerror(errno), name);
	close(fd);
	return -1;
    }
    void* ptr = mmap(NULL, sizeof(char), PROT_READ, MAP_PRIVATE, fd, 0);
    if(ptr == (void*)-1)
    {
	fprintf(stderr, "Unable to memory map file : %s\n", strerror(errno));
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
