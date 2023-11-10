#include "ioworker.h"
#include "qdatetime.h"
#include "qdebug.h"

void IOWorker::runJob()
{
    const uint8_t * pSourceData = diskSrc->getMMappedReadAddr();
    uint64_t lastReportTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    uint64_t alreadyWritten = 0;
    aborted = false;
    finished = false;

    while (alreadyWritten + 1024 * 1024 < cpLen && !aborted) // write 1mb at a time
    {
        uint64_t written = diskDst->writeRaw(offDst + alreadyWritten, 1024 * 1024, pSourceData + offSrc + alreadyWritten);
        alreadyWritten += written;
        if (written == 0)
        {
            qDebug() << "Error during write!";
            aborted = true;
        }
        uint64_t currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        if (currentTime - lastReportTime > 1000)
        {
            emit copyProgress(alreadyWritten);
            lastReportTime = currentTime;
        }

    }
    if (!aborted)
    {
        alreadyWritten += diskDst->writeRaw(offDst + alreadyWritten, cpLen - alreadyWritten, pSourceData + offSrc + alreadyWritten);
        aborted = alreadyWritten != cpLen;
    }

    finished = true;
    emit copyFinished(alreadyWritten);



}

bool IOWorker::isValid()
{
    return  diskSrc->isValid() && diskDst->isValid() && diskSrc->getMMappedReadAddr() != 0;
}
