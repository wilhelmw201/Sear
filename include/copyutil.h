#ifndef COPYUTIL_H
#define COPYUTIL_H
#include <memory>
#include "anydisk.h"
#include <QObject>
class CopyUtilImpl;

class CopyUtil: public QObject
{
    Q_OBJECT
private:
//    std::unique_ptr<CopyUtilImpl> impl;
    const uint8_t* pSourceData;
    void continueCopy();
    bool isHalted;
    bool valid;
    bool isFinished;
    AnyDisk* diskDst;
    uint64_t offSrc;
    uint64_t offDst;
    uint64_t len;
public:
    // the caller is responsible for ptr.
    CopyUtil(AnyDisk* diskSrc, AnyDisk* diskDst);
    ~CopyUtil();
    void setTask(uint64_t offSrc, uint64_t offDst, uint64_t len);
    uint64_t doCopy();
    bool isCopyOngoing(){return isFinished;}
    bool isAborted(){return isHalted;}
    bool isValid();
signals:
    void copyProgress(uint64_t copied);
    void copyFinished(uint64_t copied);
public slots:
    void stopCopy() {isHalted = true;}

};

#endif // COPYUTIL_H
