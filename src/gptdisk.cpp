#include "gptdisk.h"
#include "crc32.h"
#include <windows.h>
#include <memory>
#include <iostream>
#include <QDebug>

// create header from disk.
bool GPTDisk::createHeader()
{
    if (!isValid()) return false;
    auto buffer = std::make_unique<uint8_t[]>(getSectorSize()*100);
    memset(buffer.get(), 0, getSectorSize()*100);
    readRaw(getSectorSize(), getSectorSize()*100, buffer.get());
    memcpy(&gptHeader, buffer.get(), sizeof(GPTPrimaryHeader));
    return true;
}



GPTDisk::GPTDisk(QString name) : AnyDisk(name)
{
    // note: the constructor is moved to the createGPT fcn.
}

GPTCreateErrorCode GPTDisk::createGPT(QString path, std::unique_ptr<GPTDisk>* storage)
{
    std::unique_ptr<GPTDisk>& disk = *storage;
    // dont throw errors!!
    qDebug() << "Trying to create" << path;
    disk = std::unique_ptr<GPTDisk>(new GPTDisk(path));

    if (!disk->isValid()) {
       return GPTCreateErrorCode::ERROR_OPENING;
    }
    disk->createHeader();

    if (disk->gptHeader.signature != 0x5452415020494645ULL)
    {
       return GPTCreateErrorCode::ERROR_NOT_GPT;
    }

    qDebug() << "Header Read Done";

    // read partitions
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(disk->getSectorSize());
    for (int iPartition = 0; iPartition < (int)disk->gptHeader.numPartitionEntries; iPartition++)
    {
       // it seems the read must be sector aligned or it will fail
       int offset = disk->gptHeader.lbaPartitionHeaderArrayStart * disk->getSectorSize() + iPartition * 128;
       memset(buffer.get(), 0, 512);
       disk->readRaw(offset/512*512, 512, buffer.get());
       GPTPartition newPartition(buffer.get()+offset%512);
       if (newPartition.getLBCount() == 0) break; //continue
       disk->gptPartitions.push_back(newPartition);


       //auto header = newPartition.getHeader();
       //qDebug() << "Read Partition" << newPartition.getName();
       //qDebug() << "From" << header->firstPartitionLBA << "To" << header->lastPartitionLBA;
       //qDebug() << "Size" << newPartition.getLBCount() / 1024.0 * disk->getSectorSize() / 1024.0 << "MB";

    }
    disk->tblBackup = std::move(buffer);

    // check CRC32
    std::unique_ptr<uint8_t[]> bufferHeader = std::make_unique<uint8_t[]>(disk->getSectorSize());
    disk->readRaw(512, 512, bufferHeader.get());
    uint32_t crcHeaderGot = *(uint32_t*)(bufferHeader.get()+16); // little endian
    uint32_t crcPartitionGot = *(uint32_t*)(bufferHeader.get()+88); // little endian
    memset(bufferHeader.get()+16, 0, 4);
    std::unique_ptr<uint8_t[]> bufferAllPartitions = std::make_unique<uint8_t[]>((disk->gptHeader.numPartitionEntries)*128);
    disk->readRaw(1024, (disk->gptHeader.numPartitionEntries)*128, bufferAllPartitions.get());
    uint32_t crcHeaderExpected = crc32buf(bufferHeader.get(), 92);
    uint32_t crcPartitionExpected = crc32buf(bufferAllPartitions.get(), (disk->gptHeader.numPartitionEntries)*128);
    if (crcHeaderExpected != crcHeaderGot || crcPartitionExpected != crcPartitionGot)
    {
        return GPTCreateErrorCode::ERROR_HEADER_INTEGRITY;
    }

   return GPTCreateErrorCode::ERROR_NONE;
}

bool GPTDisk::checkCRC() const
{
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(getSectorSize()*2 + 128*128);

}



GPTDisk::~GPTDisk()
{


}




