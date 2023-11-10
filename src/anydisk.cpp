#include "anydisk.h"
#include "utils.h"
#include "qdebug.h"
#include "windows.h"

#include <QFileInfo>

class AnyDiskImpl
{
public:
    HANDLE hDevice;
    QString diskLabel;
    int sectorSize;
    uint64_t diskSize;
    uint64_t sectorCount;
    uint8_t* mmappedFile;
    HANDLE hSourceMapping;

    //        CloseHandle(hSourceFile);

    AnyDiskImpl(QString name)
    {
        hDevice = CreateFile(name.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (name[0] != '\\')
        {
            // looks like a normal file
            sectorSize = 512;
            QFileInfo fileInfo(name);
            diskLabel = fileInfo.fileName();
            diskSize = fileInfo.size();
        }
        else
        {
            initDiskLabel();
            initDiskSize();
        }
        mmappedFile = 0;

    }
    ~AnyDiskImpl()
    {
        if (mmappedFile)
        {
            UnmapViewOfFile(mmappedFile);
            CloseHandle(hSourceMapping);
        }


        CloseHandle(hDevice);
    }
    uint64_t readRaw(uint64_t offset, uint64_t count, uint8_t * buffer)
    {
        // remember to align your read!
        if (INVALID_HANDLE_VALUE == hDevice)
        {
            throw std::runtime_error("reading an invalid disk");
        }

        LARGE_INTEGER offsetWindows;
        offsetWindows.QuadPart = offset;
        unsigned long bytesRead;
        if (!SetFilePointerEx(hDevice, offsetWindows, 0, FILE_BEGIN))
        {
            auto err = GetLastError();
            qDebug() << "Seek failed. Error " << err;
            return 0;
        }
        if (!ReadFile(hDevice, buffer, count, &bytesRead, NULL))
        {
            auto err = GetLastError();
            qDebug() << "Read failed. Error " << err;
            if (offset % sectorSize != 0)
            {
                qDebug() << "remember to align the read to sector";
            }
            return 0;
        }
        return (int)bytesRead;
    }
    uint64_t writeRaw(uint64_t offset, uint64_t count, const uint8_t *buffer)
    {
        DWORD bytesWritten;
         LARGE_INTEGER moveDistance;
         moveDistance.QuadPart = offset;

         if (!SetFilePointerEx(hDevice, moveDistance, NULL, FILE_BEGIN)) {
             auto err = GetLastError();
             qDebug() << "Error setting File Ptr of dest:" << err ;
             return 0;
         }
         if (WriteFile(hDevice, (uint8_t*)buffer, count, &bytesWritten, NULL) == FALSE)
         {
             auto err = GetLastError();
             qDebug() << "Error writefile:" <<  err;
             return 0;
         }
         return bytesWritten;
    }

    void doMMapping()
    {
        // Get the size of the source file


        // Create a mapping of the source file
        hSourceMapping = CreateFileMapping(
            hDevice,                    // Source file handle
            NULL,                           // Security attributes (default)
            PAGE_READONLY,                  // Protection (read-only)
            0,                              // Maximum size high-order
            0,                              // Maximum size low-order
            NULL                            // Name (not used)
        );

        if (hSourceMapping == NULL) {
            // Handle error
            auto err = GetLastError();
            qDebug() << "Error: " << err;

            return;
        }
        qDebug() << "mmapping: " << diskSize << "bytes";
        // Map the source file into memory
        mmappedFile = (uint8_t*)MapViewOfFile(
            hSourceMapping,                 // File mapping handle
            FILE_MAP_READ,                  // Desired access (read)
            0,                              // File offset high-order
            0,                              // File offset low-order
            diskSize                        // Number of bytes to map
        );

        if (mmappedFile == NULL) {
            auto err = GetLastError();
            // Handle error
            CloseHandle(hSourceMapping);
            qDebug() << "Error: " << err;

            return;
        }

    }
private:

    void initDiskSize()
    {
        diskSize = 0;
        sectorSize = 0;
        sectorCount = 0;
        if (hDevice == INVALID_HANDLE_VALUE) {
            return;
        }
        // Get the disk geometry
        DISK_GEOMETRY diskGeometry;
        DWORD bytesReturned;
        if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
            diskSize = (uint64_t)diskGeometry.Cylinders.QuadPart * diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack * diskGeometry.BytesPerSector;
            sectorCount = (uint64_t)diskGeometry.Cylinders.QuadPart * diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack;
            sectorSize = diskGeometry.BytesPerSector;
            if (diskSize != sectorCount * sectorSize)
            {
                qDebug() << "strange diskSize";
            }
        }
    }
    void initDiskLabel()
    {
        diskLabel = "";
        STORAGE_PROPERTY_QUERY storagePropertyQuery;
        storagePropertyQuery.PropertyId = StorageDeviceProperty;
        storagePropertyQuery.QueryType = PropertyStandardQuery;

        STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = {  };
        DWORD bytesReturned = 0;

        if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery),
            &storageDescriptorHeader, sizeof(storageDescriptorHeader), &bytesReturned, NULL)) {

            // Now that we have the storage descriptor, allocate memory for the full property query result.
            DWORD dwSize = storageDescriptorHeader.Size;
            BYTE* pOutBuffer = new BYTE[dwSize];

            if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(storagePropertyQuery),
                pOutBuffer, dwSize, &bytesReturned, NULL)) {
                STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
                LPSTR vendor;
                LPSTR product;
                if (pDeviceDescriptor->VendorIdOffset > 0)
                {
                    vendor = (LPSTR)pOutBuffer + pDeviceDescriptor->VendorIdOffset;
                    diskLabel += QString(vendor);
                }
                if (pDeviceDescriptor->ProductIdOffset > 0) {
                    product = (LPSTR)pOutBuffer + pDeviceDescriptor->ProductIdOffset;
                    diskLabel += QString(product);
                }
            }
        }
    }

};

AnyDisk::AnyDisk(QString name)
{
    this->impl = std::make_unique<AnyDiskImpl>(name);
    devPath = name;
}

AnyDisk::~AnyDisk()
{

}

QString AnyDisk::getDiskLabel() const
{
    return impl->diskLabel;
}

int AnyDisk::getSectorSize() const
{
    return impl->sectorSize;
}

int AnyDisk::getSectorCount() const
{
    return impl->sectorCount;
}

uint64_t AnyDisk::readRaw(uint64_t offset, uint64_t count, uint8_t *buffer) const
{
    return impl->readRaw(offset, count, buffer);
}

uint64_t AnyDisk::writeRaw(uint64_t offset, uint64_t count, const uint8_t *buffer)
{
    return impl->writeRaw(offset, count, buffer);

}

int AnyDisk::isValid() const
{
    return impl->diskSize > 0;
}

uint64_t AnyDisk::getSize() const
{
    return impl->diskSize;
}

const uint8_t *AnyDisk::getMMappedReadAddr()
{
    if (impl->mmappedFile == 0)
    {
        impl->doMMapping();
    }
    return impl->mmappedFile;
}

