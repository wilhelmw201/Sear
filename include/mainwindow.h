#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gptdisk.h"
#include <QFuture>
#include <QMap>
#include <memory>
#include <vector>
#include "copyutil.h"
#include "ioworker.h"
#include "qthread.h"

// Q_DECLARE_METATYPE(uint64_t)

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_selectFileButton_clicked();
    void on_selectDriveButton_clicked();
    void on_startButton_clicked();

    void copyImageFinished(uint64_t copied);
    void copyImageProgress(uint64_t copied);
    void on_stopButton_clicked();

private:
    Ui::MainWindow *ui;
    int selectedGPTDiskIdx = -1;
    // QVector does not agree with unique ptr somehow
    std::vector<std::unique_ptr<GPTDisk>> disks;
    QMap<QString, QString> invalidDisks;
    QFuture<uint64_t> copyResult;
    std::unique_ptr<GPTDisk> isoDisk;
    //std::unique_ptr<CopyUtil> cpUtil;
    uint64_t lastProgressReportTime;
    void discoverDrives();
    std::unique_ptr<uint8_t[]> draftedTable;
    std::unique_ptr<QThread> copyThread;
    IOWorker worker;
};
#endif // MAINWINDOW_H
