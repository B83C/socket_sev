#include "readahead.h"

static struct readahead_t* read_dym;
static int counter;

static void retrievepaths(DIR* d, struct file_cache* pool, struct libdeflate_compressor* compressor)
{
    struct dirent* temp;

    while((temp = readdir(d)) != NULL)
    {
	if(temp->d_name[0] == '.') continue;
	if(temp->d_type == DT_DIR)
	{
	    char* prevbase = read_dym->basepointer;
	    read_dym->basepointer = memccpy(memccpy(read_dym->basepointer, temp->d_name, '\0', strlen(temp->d_name) + 1) - 1, "/\0", '\0', 2) - 1;
	    int tmpfd = openat(read_dym->__dirfd, read_dym->base, O_DIRECTORY);
	    DIR* tmp = fdopendir(tmpfd);
	    retrievepaths(tmp, pool, compressor);
	    close(tmpfd);
	    closedir(tmp);
	    read_dym->basepointer = prevbase;
	    *(read_dym->basepointer) = 0;
	}
	else if(temp->d_type == DT_REG)
	{
	    register int baselen = strlen(read_dym->base);
	    register int filenamelen = strlen(temp->d_name); 
	    if(counter >= MAX_INITIAL_FILENUM)
	    {
		pool->entry_len += 10;
		realloc(pool->filenames, pool->entry_len);
	    }
	    pool->filenames[counter] = malloc(baselen + filenamelen);
	    memccpy(memccpy(pool->filenames[counter], read_dym->base, '\0', baselen + 1) - 1, temp->d_name, '\0', filenamelen + 1);
	    struct stat st;
	    int file = openat(read_dym->__dirfd, pool->filenames[counter],  O_DIRECT | O_RDONLY);
	    fstat(file, &st);
	    char* uncompressed =  mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, file, 0);
	    close(file);
	    char* compressed = malloc(st.st_size);
	    size_t compressed_size = libdeflate_deflate_compress(compressor, uncompressed, st.st_size, compressed, st.st_size);
	    if(!compressed_size)
	    {
		fprintf(stderr, "Unable to compress %s\n", pool->filenames[counter]);
		free (compressed);
		pool->data[counter].data = uncompressed;
		pool->data[counter].data_len = st.st_size;
	    }
	    else
	    {
		realloc(compressed, compressed_size);
		pool->data[counter].data = compressed;
		pool->data[counter].data_len = compressed_size;
		munmap(uncompressed, st.st_size);
	    }
	    counter++;
	}
    }
}

void readahead_init(int dirfd_, struct file_cache* pool,cmph_t **mphash)
{
    struct libdeflate_compressor* compressor = libdeflate_alloc_compressor(DEFLATE_COMPRESSION_LEVEL);
    DIR* d = fdopendir(dirfd_);
    read_dym = malloc(sizeof(struct readahead_t));
    read_dym->basepointer = read_dym->base;
    read_dym->__dirfd = dirfd_;
    retrievepaths(d, pool, compressor);
    closedir(d);
    free(read_dym);

    //cmph_io_adapter_t* source = cmph_io_struct_vector_adapter(pool->filenames, sizeof(char**), 0, 0,  counter);
    cmph_io_adapter_t* source = cmph_io_vector_adapter(pool->filenames,  counter);
    cmph_config_t *config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_CHM);
    *mphash = cmph_new(config);
    cmph_io_vector_adapter_destroy(source);
    cmph_config_destroy(config);
}

struct file_data* retrievefile(cmph_t* hash, struct file_cache* pool, char* path, size_t len)
{
    int i = cmph_search(hash, path, len);
    if(strncmp(pool->filenames[i], path, len))
    {
	return -1;
    }
    return &pool->data[i];
}
