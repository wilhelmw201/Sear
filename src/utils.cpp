#include "utils.h"
#include "crc32.h"
#include "qdebug.h"
#include "windows.h"
#include <QSet>
#include <QFileInfo>
#include <QCryptographicHash>

#include <iostream>
#include <fstream>

QString sizeToHumanReadable(uint64_t sizeBytes)
{
    if (sizeBytes < (uint64_t)1024*10) // 10kb
    {
        return QString("%1B").arg((double)sizeBytes, 0, 'f', 2);
    }
    if (sizeBytes < (uint64_t)1024*1024*10) // 10mb
    {
        return QString("%1KB").arg(sizeBytes/1024.0, 0, 'f', 2);
    }
    if (sizeBytes < (uint64_t)1024*1024*1024*10) // 10gb
    {
        return QString("%1MB").arg(sizeBytes/1024.0/1024.0, 0, 'f', 2);
    }
    return QString("%1GB").arg(sizeBytes/1024.0/1024.0/1024.0, 0, 'f', 2);
}

DraftPartitionTableError draftGPTHeaderSectors(GPTDisk *isoDisk, GPTDisk *tgtDisk, uint8_t *buffer, bool regenUUID)
{

    QSet<QByteArray> seenUUIDs;
    // check if there is a uuid conflict.
    for (const GPTPartition& partition: isoDisk->getPartitions())
    {
        seenUUIDs.insert(QByteArray((const char*)partition.getUUIDRaw(), 16));
    }
    for (const GPTPartition& partition: tgtDisk->getPartitions())
    {
        if (seenUUIDs.contains(QByteArray((const char*)partition.getUUIDRaw(), 16)))
        {
            qDebug() << QObject::tr("Partition with GUID %1 found both on target and source.")
                        .arg(UUIDMixedEndianToUUIDString(partition.getUUIDRaw()));
            return DraftPartitionTableError::ERROR_GUID_CONFLICT;
            // TODO on demand, rename the partition on tgtdisk
        }
    }
    // wipe buffer
    int sectorSize = tgtDisk->getSectorSize();
    memset(buffer, 0, 512+512+128*128);

    // copy the MBR of image disk
    isoDisk->readRaw(0, 512, buffer);

    // merge the partition tables.
    QVector<const GPTPartitionHeader*> partitionHeadersAll;
    for (const auto& partitionImg: isoDisk->getPartitions())
    {
        partitionHeadersAll.append(partitionImg.getHeader());
    }
    for (const auto& partitionImg: tgtDisk->getPartitions())
    {
        partitionHeadersAll.append(partitionImg.getHeader());
    }
    makePartitionHeader(partitionHeadersAll, buffer + 1024);
    makeHeader(*isoDisk->getHeader(),
               buffer + 1024,

               tgtDisk->getSize()/tgtDisk->getSectorSize() ,
               buffer+512);
    // TODO fix mbr


    // debug
    std::fstream fs;
    fs.open("C:\\test.out", std::ios::out | std::ios::binary | std::ios::trunc);
    fs.write((char*)buffer, 34 * sectorSize);
    fs.close();
    // done!
    return DraftPartitionTableError::ERROR_NONE;
}

QString UUIDMixedEndianToUUIDString(const uint8_t *source)
{
    QString result = "";
    int sequence[] = {3,2,1,0,-1,5,4,-1,7,6,-1,8,9,-1,10,11,12,13,14,15};
    for (int i = 0; i < (int)(sizeof(sequence)/sizeof(int)); i++)
    {
        if (sequence[i] == -1)
        {
            result += "-";
        }
        else
        {
            result += QString::number(source[sequence[i]], 16).rightJustified(2, '0').toLower();
        }
    }
    return result;
}

void makePartitionHeader(const QVector<const GPTPartitionHeader*>& partitions, uint8_t *buffer)
{
    memset(buffer, 0, partitions.size() * 128);
    for (int i = 0; i < partitions.size(); i++)
    {
        memcpy(buffer + 128 * i, partitions[i], 128);
    }
}

void makeHeader(const GPTPrimaryHeader &mainHeader, const uint8_t *partitionBuffer, uint64_t diskLBACount, uint8_t *buffer)
{
    memset(buffer, 0, 512);
    GPTPrimaryHeader myHeader = mainHeader;
    myHeader.headerCRC = 0;
    myHeader.headerLBA = 1;
    myHeader.backupHeaderLBA = diskLBACount - 1;
    myHeader.firstDataLBA = 34;
    myHeader.lastDataLBA = diskLBACount - 34;
    myHeader.lbaPartitionHeaderArrayStart = 2;
    myHeader.numPartitionEntries = 128;
    myHeader.partitionEntrySize = 128;
    myHeader.partitionCRC32 = crc32buf(partitionBuffer, 128 * 128);
    memcpy(buffer, &myHeader, sizeof(myHeader));
    //
    ((GPTPrimaryHeader*)buffer)->headerCRC = crc32buf(buffer, 92);

}

uint64_t diskCopy(GPTDisk *tgtDisk, GPTDisk *srcDisk, uint64_t tgtOff, uint64_t srcOff, uint64_t len)
{
    // blocking copy, when performance really doesnt matter.
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    srcDisk->readRaw(srcOff, len, buffer.get());
    return tgtDisk->writeRaw(tgtOff, len, buffer.get());
}
