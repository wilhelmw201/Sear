#ifndef GPTDISK_H
#define GPTDISK_H

#include <QVector>
#include <memory>
#include "gptpartition.h"
#include "anydisk.h"
#pragma pack(push,1)
struct GPTPrimaryHeader
{
    uint64_t signature;
    uint32_t uefiRevision;
    uint32_t gptHeadersize;
    uint32_t headerCRC;
    uint32_t reserved1;
    uint64_t headerLBA;
    uint64_t backupHeaderLBA;
    uint64_t firstDataLBA;
    uint64_t lastDataLBA;
    uint8_t uuidRaw[16]; // this is aligned
    uint64_t lbaPartitionHeaderArrayStart;
    uint32_t numPartitionEntries;
    uint32_t partitionEntrySize;
    uint32_t partitionCRC32;
} ;
#pragma pack(pop)
static_assert(sizeof(GPTPrimaryHeader) == 92);

enum class GPTCreateErrorCode{
    ERROR_NONE = 0,
    ERROR_OPENING,
    ERROR_NOT_GPT,
    ERROR_HEADER_INTEGRITY,
};

class GPTDiskImpl;
class GPTDisk: public AnyDisk
{
private:
    bool createHeader();
    GPTPrimaryHeader gptHeader;
    QVector<GPTPartition> gptPartitions;
    std::unique_ptr<uint8_t[]> tblBackup;
    GPTDisk(QString name);
public:
    // storage must be empty when passed in. if the error code is not 0,
    // the result may be a partially initialized GPTDisk.
    static GPTCreateErrorCode createGPT(QString path, std::unique_ptr<GPTDisk>* storage);
    const QVector<GPTPartition>& getPartitions() const {return gptPartitions;}
    const uint8_t* getUUIDRaw() const {return gptHeader.uuidRaw;}
    const uint8_t* getGPTTblBackup() const {return tblBackup.get();}
    const GPTPrimaryHeader* getHeader() {return &gptHeader;}
    bool checkCRC() const;
    ~GPTDisk();
    int readSector(int sector, uint8_t* buffer);
};

#endif // GPTDISK_H
