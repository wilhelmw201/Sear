#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <memory>
#include "gptdisk.h"
#include <QVector>
#include <QMetaType>



QString sizeToHumanReadable(uint64_t sizeBytes);
enum class DraftPartitionTableError
{
    ERROR_NONE = 0,
    ERROR_INSUFFICIENT_ENTRIES,
    ERROR_GUID_CONFLICT,
    ERROR_DISK_SIZE,
};
DraftPartitionTableError draftGPTHeaderSectors(GPTDisk* isoDisk, GPTDisk* tgtDisk, uint8_t * buffer, bool regenUUID=false);
// make header. buffer is just 1 sector.
// buffer must be at least 512 bytes in size
void makeHeader(const GPTPrimaryHeader& mainHeader, const uint8_t* partitionBuffer, uint64_t diskLBACount,  uint8_t* buffer);
void makePartitionHeader(const QVector<const GPTPartitionHeader*>& partitions,  uint8_t* buffer);
// blocking copy
uint64_t diskCopy(GPTDisk* tgtDisk, GPTDisk* srcDisk, uint64_t tgtOff, uint64_t srcOff, uint64_t len);
QString UUIDMixedEndianToUUIDString(const uint8_t* source);


#endif // UTILS_H
