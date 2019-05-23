#include "hash.h"

#define SEED_LENGTH 65

typedef uint64_t Hash;

const char           key_seed[SEED_LENGTH] = "b4967483cf3fa84a3a233208c129471ebc49bdd3176c8fb7a2c50720eb349461";
const unsigned short *key_seed_num         = (unsigned short*)key_seed;

Hash getHash(const char *word, size_t length)
{
    Hash hash = 0;
    
    for (off_t i = 0; i < length; i++)
    {
        Hash num_char = (Hash)word[i];
        Hash seed     = (Hash)key_seed_num[(i % SEED_LENGTH)];
        
        hash += num_char * seed * (i + 1);
    }
    
    return hash;
}
