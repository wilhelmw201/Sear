#include "copyutil.h"
#include "qdebug.h"
//class CopyUtilImpl {
//public:

//    const uint8_t* pSourceData;
//    AnyDisk* diskDst; // this should live longer than CopyUtilImpl.

//    bool valid;
//    CopyUtilImpl(AnyDisk *diskSrc, AnyDisk *diskDst)
//    {
//        this->diskDst = diskDst;

//        valid = false;
//        pSourceData = diskSrc->getMMappedReadAddr();
//        if (pSourceData)
//        {
//            valid = true;
//        }


//    }
//    ~CopyUtilImpl(){
////        UnmapViewOfFile(pSourceData);
////        CloseHandle(hSourceMapping);
////        CloseHandle(hSourceFile);

//    }
//    uint64_t doCopy(uint64_t offSrc, uint64_t offDst, uint64_t len) {
//        if (!valid) throw std::runtime_error("doCopy with invalid CopyUtil instance");
////        DWORD bytesWritten;
////        LARGE_INTEGER moveDistance;
////        moveDistance.QuadPart = offDst;
//        return diskDst->writeRaw(offDst, len, (uint8_t*)pSourceData+offSrc);
////        if (!SetFilePointerEx(hDestFile, moveDistance, NULL, FILE_BEGIN)) {
////            qDebug() << "Error setting File Ptr of dest:" << (int)GetLastError() ;
////            return 0;
////        }
////        if (WriteFile(hDestFile, (uint8_t*)pSourceData+offSrc, len, &bytesWritten, NULL) == FALSE)
////        {
////            return 0;
////        }
////        return bytesWritten;
//    }
//};




CopyUtil::CopyUtil(AnyDisk *diskSrc, AnyDisk *diskDst)
{
    this->diskDst = diskDst;
    isHalted = false;
    isFinished = true;
    pSourceData = diskSrc->getMMappedReadAddr();
    valid = pSourceData != 0;
}

CopyUtil::~CopyUtil()
{

}

void CopyUtil::setTask(uint64_t offSrc, uint64_t offDst, uint64_t len)
{
    this->offDst = offDst;
    this->offSrc = offSrc;
    this->len = len;
}



uint64_t CopyUtil::doCopy()
{
    uint64_t alreadyWritten = 0;
    isHalted = false;
    isFinished = false;
//    while (alreadyWritten + 1024 * 1024 < len && !isHalted) // write 1mb at a time
//    {
//        int written = diskDst->writeRaw(offDst + alreadyWritten, 1024 * 1024, pSourceData + offSrc + alreadyWritten);
//        alreadyWritten += written;
//        if (written == 0)
//        {
//            qDebug() << "Error during write!";
//            isHalted = true;
//        }
//        emit copyProgress(alreadyWritten);
//    }
//    if (!isHalted)
//    {
//        alreadyWritten += diskDst->writeRaw(offDst + alreadyWritten, len - alreadyWritten, pSourceData + offSrc + alreadyWritten);
//        isHalted = alreadyWritten == len;
//    }



    isFinished = true;
    emit copyFinished(alreadyWritten);

    return alreadyWritten;
}

bool CopyUtil::isValid()
{
    return valid;
}
