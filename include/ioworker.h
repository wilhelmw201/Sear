#ifndef IOWORKER_H
#define IOWORKER_H

#include "gptdisk.h"
#include <QThread>
#include <QObject>
// this class is responsible for copying the image to target disk
// shall run in a seperate thread, so it is fine to block.
class IOWorker : public QObject
{
    Q_OBJECT
public:
    explicit IOWorker(QObject *parent = nullptr):QObject(parent),aborted(false),finished(true),diskSrc(0),diskDst(0){}
    ~IOWorker(){}
    void runJob();
    void setJob(GPTDisk* src, GPTDisk* dst, uint64_t offsetSrc, uint64_t offsetDst, uint64_t len)
        {diskSrc=src; diskDst=dst;cpLen=len;offSrc=offsetSrc;offDst=offsetDst;}
    void stopCopy() {aborted = true;}
    bool isFinished(){return finished;}
    bool isWithError(){return aborted;}
    bool isValid();
signals:
//    void statusUpdate(QString msg);
    void copyProgress(uint64_t copied);
    void copyFinished(uint64_t copied);

private:
    volatile bool aborted;
    volatile bool finished;
    uint64_t cpLen;
    uint64_t offSrc, offDst;
    GPTDisk* diskSrc;
    GPTDisk* diskDst;

};

#endif // IOWORKER_H
