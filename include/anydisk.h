#ifndef ANYDISK_H
#define ANYDISK_H
#include <QString>
#include <memory>


class AnyDiskImpl; // anything that include platform specific stuff go here
class AnyDisk // this can also be a file.
{
public:
    AnyDisk(QString devPath);
    ~AnyDisk();

    QString getPartitionSizeHumanReadable() const;
    //uint64_t getPartitionSizeBytes();
    QString getDiskLabel() const; // get Device Name, or a human readable name for the disk

    int getSectorSize() const;
    int getSectorCount() const;
    uint64_t readRaw(uint64_t offset, uint64_t count, uint8_t * buffer) const;
    uint64_t writeRaw(uint64_t offset, uint64_t count, const uint8_t * buffer);
    int isValid() const; // returns if the handle is opened correctly
    uint64_t getSize() const;
    QString getDevPath() const {return devPath;}
    const uint8_t * getMMappedReadAddr(); // probably windows specific

private:
    std::unique_ptr<AnyDiskImpl> impl;
    QString devPath;
};

#endif // ANYDISK_H
