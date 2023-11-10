#ifndef GPTPARTITION_H
#define GPTPARTITION_H

#include <cstdint>
#include <QString>
#include <memory>

#pragma pack(push,1)
struct GPTPartitionHeader
{
    uint8_t uuidTypeRaw[16];
    uint8_t uuidPartitionRaw[16];
    uint64_t firstPartitionLBA;
    uint64_t lastPartitionLBA; // inclusive, usually odd
    uint64_t attribFlags;
    uint8_t name[72];
} ;
#pragma pack(pop)
static_assert(sizeof(GPTPartitionHeader) == 56+72);

class GPTPartition
{
public:
    GPTPartition(uint8_t* buffer); // create from buffer
    const GPTPartitionHeader* getHeader ()const {return &gptPartitionHeader;}
    const QString getName() const {return QString::fromWCharArray((wchar_t*)gptPartitionHeader.name);}
    uint64_t getLBCount() const {return gptPartitionHeader.lastPartitionLBA - gptPartitionHeader.firstPartitionLBA; }
    uint64_t getFirstLB() const {return gptPartitionHeader.firstPartitionLBA;}
    uint64_t getLastLB() const {return gptPartitionHeader.lastPartitionLBA; }
    const uint8_t* getUUIDRaw() const {return gptPartitionHeader.uuidPartitionRaw;}

private:
    GPTPartitionHeader gptPartitionHeader;

};

#endif // GPTPARTITION_H
