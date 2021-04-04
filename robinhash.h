struct member_t
{
    uint32_t key;
    uint32_t phone_num;
};

struct bucket_t
{
    uint32_t key;
    uint16_t val : 16;
    uint8_t flags : 8;
    uint8_t distance : 8;
};

struct robinhash_t
{
    struct bucket_t* hashtable;
    struct member_t* table;
    uint32_t capacity;
    uint32_t table_size;
    uint16_t table_tail;
    uint16_t key_num;
};

__attribute__((always_inline))
void new_robinhash_t(struct robinhash_t* rh_t, struct member_t* table, struct bucket_t* hashtable, uint32_t capacity, uint32_t table_size)
{
    rh_t->capacity = capacity;
    rh_t->table_size = table_size;
    rh_t->table = table;
    rh_t->hashtable = hashtable;
    rh_t->key_num = 0;
    rh_t->table_tail = 0;
}

uint16_t robin_lookup(struct robinhash_t* rh_t, uint32_t key)
{
    int index = MODULO(XXH32(&key, 4, seed), rh_t->capacity);
    struct bucket_t* entry = rh_t->hashtable + index;
    for(int distance = 0; (entry->distance) >= distance; distance++, entry++)
    {
	if(key == entry->key && !(entry->flags & KEY_DELETED))
	{
	    return entry->val;
	}
    }
    return -1;
}

struct member_t* member_lookup(struct robinhash_t* rh_t, uint32_t key)
{
    int index = MODULO(XXH32(&key, 4, seed), rh_t->capacity);
    struct bucket_t* entry = rh_t->hashtable + index;
    for(int distance = 0; (entry->distance) >= distance; distance++, entry++)
    {
	    //printf("probing key : %x at %d\n", key, index + distance);
	if(key == entry->key && !(entry->flags & KEY_DELETED))
	{
	    return rh_t->table + entry->val;
	}
    }
    return -1;
}

#define ERR_EXIST 1

    static inline
int robin_insert_unsafe(struct robinhash_t* rh_t, uint32_t key, uint16_t val)
{
    int index = MODULO(XXH32(&key, 4, seed), rh_t->capacity);
    struct bucket_t* entry = rh_t->hashtable + index;
    int distance = 0;
    int entry_dist;
restart:
    //printf("Inserting %x %d at %d\n", key, val, index);
    entry_dist = entry->distance;
    if(key && entry->key && !(entry->flags & KEY_DELETED))
    {
	if(distance > entry_dist)
	{
	    uint32_t temp_key = entry->key;
	    uint32_t temp_val = entry->val;
	    uint32_t temp_dist = entry_dist;

	    //   printf("rich key found, key %x , val %d , distance %d to key %x , val %d , distance %d\n", key, val, distance, temp_key, temp_val, temp_dist);
	    *(uint64_t*)entry = (uint64_t)key | ((uint64_t)val << 32) | ((uint64_t)distance << 56);

	    key = temp_key;
	    val = temp_val;
	    distance = temp_dist;
	}
	distance++;
	index = MODULO(++index, rh_t->capacity);
	entry = rh_t->hashtable + index;
	goto restart;
    }
    if(entry->key == key) return ERR_EXIST;
    *(uint64_t*)entry = (uint64_t)key | ((uint64_t)val << 32) | ((uint64_t)distance << 56);
    return 0;
}


    static inline
void robin_rehash(struct robinhash_t* rh_t, uint32_t newsize)
{
    rand(seed);
    int key_num = rh_t->key_num;
    rh_t->capacity = ROUND(newsize);
    //printf("New capacity : %d\n", rh_t->capacity);
    rh_t->hashtable = realloc(rh_t->hashtable, rh_t->capacity * sizeof(struct bucket_t));
    memset(rh_t->hashtable, 0, rh_t->capacity * sizeof(struct bucket_t));
    for(int i = 0; i < rh_t->table_size && key_num > 0; i++)
    {
	int key = (rh_t->table + i)->key; 
	if(key == NULL) continue;
	//printf("Reinserting %x, val %d\n", key, i );
	robin_insert_unsafe(rh_t, key, i);
	key_num--;
    }
}

int member_insert(struct robinhash_t* rh_t, struct member_t* member)
{
    if(APPROX_875_PERCENT(rh_t->key_num, rh_t->capacity))
    {
        robin_rehash(rh_t, rh_t->capacity << 1);
    }
    int val;
    if(ring_buf_tail != ring_buf_head)
    {
	val = *(ring_buf + ring_buf_head);
	ring_buf_head = MODULO(++ring_buf_head, rh_t->table_size);
    }
    else
    {
	val = rh_t->table_tail++;
    }
    *(rh_t->table + val) = *member;
    robin_insert_unsafe(rh_t, member->key, val);
    rh_t->key_num++;
}

int robin_insert(struct robinhash_t* rh_t, uint32_t key, uint16_t val)
{
    if(APPROX_875_PERCENT(rh_t->key_num, rh_t->capacity))
    {
        robin_rehash(rh_t, rh_t->capacity << 1);
    }
    robin_insert_unsafe(rh_t, key, val);
    rh_t->key_num++;
}

uint16_t robin_del(struct robinhash_t* rh_t, uint32_t key)
{
    int index = MODULO(XXH32(&key, 4, seed), rh_t->capacity);
    struct bucket_t* entry = rh_t->hashtable + index;
    for(int distance = 0; entry->distance >= distance; distance++, entry++)
    {
	if(key == entry->key)
	{
	    entry->flags |= KEY_DELETED;
	    if((rh_t->table + entry->val)->key != NULL)
	    {
		*(ring_buf + ring_buf_tail) = entry->val;
		ring_buf_tail = MODULO(++ring_buf_tail, rh_t->table_size);
	    }
	    rh_t->key_num--;
	    return 1;
	}
    }
    return -1;

}
