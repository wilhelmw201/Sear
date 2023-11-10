#include "gptpartition.h"
#include <cstring>

GPTPartition::GPTPartition(uint8_t * buffer)
{
    memcpy(&gptPartitionHeader, buffer,  sizeof(gptPartitionHeader));
}
