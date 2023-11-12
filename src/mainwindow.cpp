#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <algorithm>
#include <QtConcurrent/QtConcurrent>
#include <QScreen>
#define MAX_PROGRESS 32768

static void scaleTextByDpi(QWidget* widget)
{
    int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();

    QFont font = widget->font();
    int scaledFontSize = static_cast<int>(font.pointSize() * dpi / 96.0);
    font.setPointSize(scaledFontSize);
    widget->setFont(font);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)


{
    ui->setupUi(this);
    selectedGPTDiskIdx = -1;

    ui->selectFilePathEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // deal with the problem that higher resolution make the text tiny
    scaleTextByDpi(ui->selectedDriveDisplay);
    scaleTextByDpi(ui->statusDisplay);
    scaleTextByDpi(ui->selectFileButton);
    scaleTextByDpi(ui->speedLabel);
    scaleTextByDpi(ui->settingsButton);
    scaleTextByDpi(ui->selectDriveButton);
    scaleTextByDpi(ui->stopButton);
    scaleTextByDpi(ui->startButton);

    int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    resize(QSize(30000/dpi, 20000/dpi));
    setWindowTitle("Sear");
    ui->statusDisplay->append(("Sear v0.9"));
    ui->statusDisplay->append(("www.github.com/wilhelmw201/Sear"));

    // TODO : add more stuff to settings
    ui->settingsButton->hide();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_selectFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select ISO File"));
    if (!filePath.isEmpty()) {
        // Set the selected file path to the text box
        ui->selectFilePathEdit->setText(filePath);
    }
}


void MainWindow::on_selectDriveButton_clicked()
{
    if (!worker.isFinished()) return;
    discoverDrives();
    selectedGPTDiskIdx = -1;
    ui->selectedDriveDisplay->setText(tr("No disk selected!"));
    if (disks.size() + invalidDisks.size() == 0)
    {
        QMessageBox::information(
            this,
            tr("Sear"),
            tr("No disk found on this computer. Make sure you are running as admin.") );
        return;
    }
    if (disks.size() + invalidDisks.size() == 1)
    {
        QMessageBox::information(
            this,
            tr("Sear"),
            tr("Only 1 disk found. Make sure your medium is connected to the computer correctly.") );
    }

    // Create a QMessageBox
    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::NoButton);
    QVBoxLayout* layout = new QVBoxLayout(&msgBox);
    msgBox.layout()->addItem(layout);

    msgBox.setWindowTitle("Sear");
    layout->addWidget(new QLabel(tr("Please select the destination disk."), &msgBox));

    QVector<QRadioButton*> radioButtons; // QT has its own memory stuff and this is ok.
    for (int i = 0; i < (int)disks.size(); i++)
    {
        auto diskWrapper = std::make_unique<AnyDisk>(disks[i]->getDevPath());
        auto diskManuName = diskWrapper->getDiskLabel();
        auto diskHumanReadableSize = sizeToHumanReadable(diskWrapper->getSize());
        QRadioButton* rb = new QRadioButton(tr("Disk %1: %2").arg(diskManuName).arg(diskHumanReadableSize), &msgBox);
        radioButtons.push_back(rb);
        layout->addWidget(rb);
    }
    // create dummy disks
    for (const auto& diskName: invalidDisks.keys()) // qt cannot iterate dict with for each + pair
    {
        auto diskWrapper = std::make_unique<AnyDisk>(diskName);
        auto diskManuName = diskWrapper->getDiskLabel();
        auto diskHumanReadableSize = sizeToHumanReadable(diskWrapper->getSize());
        QRadioButton* rb = new QRadioButton(tr("Disk %1: %2\n%3").arg(diskManuName).arg(diskHumanReadableSize).arg(invalidDisks[diskName]), &msgBox);
        radioButtons.push_back(rb);
        layout->addWidget(rb);
        rb->setCheckable(false);
    }


    // Create a button box with OK and Cancel buttons
    QHBoxLayout* btnLayout = new QHBoxLayout(&msgBox);

    QPushButton* buttonAccept = new QPushButton(tr("OK"), &msgBox);
    btnLayout->addWidget(buttonAccept);
    QObject::connect(buttonAccept, &QPushButton::clicked, &msgBox, &QMessageBox::accept);
    QPushButton* buttonReject = new QPushButton(tr("Cancel"), &msgBox);
    btnLayout->addWidget(buttonReject);
    QObject::connect(buttonReject, &QPushButton::clicked, &msgBox, &QMessageBox::reject);

    layout->addLayout(btnLayout);

    // Show the message box
    QDialog::DialogCode result = (QDialog::DialogCode)msgBox.exec();

    // Check which radio button was selected
    if (result == QMessageBox::Accepted) {
        for (int i = 0; i < radioButtons.size(); i++)
        {
            if (radioButtons[i]->isChecked())
            {
                selectedGPTDiskIdx = i;
                // todo dont copy paste
                const auto& disk = disks[i];
                auto diskManuName = disk->getDiskLabel();
                auto diskHumanReadableSize = sizeToHumanReadable(disk->getSize());
                ui->selectedDriveDisplay->setText(tr("Disk %1: %2").arg(diskManuName).arg(diskHumanReadableSize));
            }
        }
    }
}




void MainWindow::on_startButton_clicked()
{
    if (selectedGPTDiskIdx < 0)
    {
        ui->statusDisplay->append(tr("Select a disk first! Stop."));
        return;
    }
    ui->statusDisplay->append(tr("Attempting to burn %1 onto %2...")
                              .arg(ui->selectFilePathEdit->toPlainText())
                              .arg(ui->selectedDriveDisplay->text()));
    ui->statusDisplay->append(tr("Inspecting disk image..."));

    GPTCreateErrorCode result = GPTDisk::createGPT(ui->selectFilePathEdit->toPlainText(), &isoDisk);
    if (result == GPTCreateErrorCode::ERROR_HEADER_INTEGRITY)
    {
        ui->statusDisplay->append(tr("Warning: disk image seems corrupt. You may want to check is signature."));
    }
    if (result == GPTCreateErrorCode::ERROR_OPENING)
    {
        ui->statusDisplay->append(tr("Error: Cannot open target image file. Stop."));
        return;
    }
    if (result == GPTCreateErrorCode::ERROR_NOT_GPT)
    {
        ui->statusDisplay->append(tr("Error: target file is not a GPT format disk image. Stop."));
        return;
    }

     ui->statusDisplay->append("OK.");
    ui->statusDisplay->append(tr("Inspecting target disk..."));
    std::unique_ptr<GPTDisk>& tgtDisk = disks[selectedGPTDiskIdx];
    if (!tgtDisk->isValid())
    {
        ui->statusDisplay->append(tr("Error: Cannot open target disk. Stop."));
        return;
    }
    auto firstPartition = std::min_element(tgtDisk->getPartitions().cbegin(), tgtDisk->getPartitions().cend(),
                                             [](const GPTPartition& p1, const GPTPartition& p2){return p1.getFirstLB() < p2.getFirstLB();});
    assert (firstPartition != tgtDisk->getPartitions().end());
    uint64_t firstOccupiedLBA = firstPartition->getFirstLB();
    uint64_t neededLBA = (isoDisk->getSize() + isoDisk->getSectorSize() - 1) / (isoDisk->getSectorSize());
    if (firstOccupiedLBA < neededLBA)
    {
        ui->statusDisplay->append(tr("Error: The target disk does not have enough space at the front to contain the disk image."
               "Space available: %1. Stop.").arg(sizeToHumanReadable(firstOccupiedLBA * (isoDisk->getSectorSize()))));
        return;
    }
    if (tgtDisk->getSectorSize() != 512)
    {
        ui->statusDisplay->append(tr("Error: The target disk logical sector size is not 512 bytes. This is untested and may cause data loss. Stop."));
        //ui->statusDisplay->append(tr("You can ignore this in settings. "));
        return;
    }
    ui->statusDisplay->append("OK.");
    ui->statusDisplay->append(tr("Drafting new partition table..."));
//    std::unique_ptr<uint8_t[]> newGPTBuffer(new uint8_t[34*tgtDisk->getSectorSize()]); // TODO handle case partition count > 128?
//    DraftPartitionTableError draftResult = draftPartitionTable(isoDisk.get(), tgtDisk.get(), newGPTBuffer.get());

    draftedTable = std::make_unique<uint8_t[]>(512 * 2 + 128 * 128);
    //draftGPTHeaderSectors(isoDisk.get(), tgtDisk.get(), draftedTable.get());
    DraftPartitionTableError draftResult = draftGPTHeaderSectors(isoDisk.get(), tgtDisk.get(), draftedTable.get());
    if (draftResult == DraftPartitionTableError::ERROR_GUID_CONFLICT)
    {
        ui->statusDisplay->append(tr("Error: GUID conflict. This may be due to having a bootable partition both on the source image and on the target disk. Stop."));
        return;
    }
    if (draftResult == DraftPartitionTableError::ERROR_INSUFFICIENT_ENTRIES)
    {
        ui->statusDisplay->append(tr("Error: Too many partitions. You will have more than 128 partitions in total on final disk. Stop."));
        return;
    }
    ui->statusDisplay->append("OK.");


    ui->statusDisplay->append(tr("Copying image to unoccupied space on target disk..."));

    // copy the image without the gpt stuff
    // todo: dont use numbers such as 1024, 128
    worker.setJob(isoDisk.get(), tgtDisk.get(), (isoDisk->getSectorSize())*(isoDisk->getHeader()->firstDataLBA), (isoDisk->getSectorSize())*(isoDisk->getHeader()->firstDataLBA),
                  isoDisk->getSectorSize()*(isoDisk->getHeader()->lastDataLBA - isoDisk->getHeader()->firstDataLBA));
    copyThread = std::make_unique<QThread>();
    worker.moveToThread(copyThread.get());
    connect(copyThread.get(), &QThread::started, &worker, &IOWorker::runJob);
    connect(&worker, &IOWorker::copyFinished, this, &MainWindow::copyImageFinished);
    connect(&worker, &IOWorker::copyProgress, this, &MainWindow::copyImageProgress);
    ui->copyProgressBar->setMinimum(0);
    ui->copyProgressBar->setValue(0);
    lastProgressReportTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    ui->copyProgressBar->setMaximum(MAX_PROGRESS); // some arbitrary number

    copyThread->start();
//    cpUtil = std::make_unique<CopyUtil>(isoDisk.get(), tgtDisk.get());
//    if (!cpUtil->isValid())
//    {
//        ui->statusDisplay->append(tr("Error: Cannot copy image to disk. Stop."));
//        return;
//    }
//    int skip = 512*2 + 128*128;
//    copyThread = std::make_unique<QThread>();
//    cpUtil->moveToThread(copyThread.get());
//    connect(cpUtil.get(), &CopyUtil::copyProgress, this, &MainWindow::copyImageProgress);
//    connect(cpUtil.get(), &CopyUtil::copyFinished, this, &MainWindow::copyImageFinished);
//    connect(copyThread.get(), &QThread::started, cpUtil.get(), &CopyUtil::doCopy);


//    // this operation must return immediately or ui will hang.


//    cpUtil->setTask(skip, skip, isoDisk->getSize() - skip);

//    QtConcurrent::run(
//        [&](uint64_t skip, uint64_t totSize)
//        {cpUtil->doCopy(skip, skip, totSize - skip);},
//        skip, isoDisk->getSize());

    //ui->statusDisplay->append(tr("Applying partition table..."));
    //ui->statusDisplay->append(tr("Finished."));
}

void MainWindow::copyImageFinished(uint64_t copied)
{
    ui->copyProgressBar->setMaximum(100);
    ui->copyProgressBar->setValue(100);
    if (worker.isWithError())
    {
        ui->statusDisplay->append(tr("Stopped. Your existing partitions have not been changed."));
        return;
    }
    ui->statusDisplay->append(tr("Copy image Finished."));
    ui->statusDisplay->append(tr("Copying MBR..."));
    uint64_t newcopied = 0;
    newcopied = diskCopy(isoDisk.get(), disks[selectedGPTDiskIdx].get(), 0, 0, 512);
    if (newcopied != 512)
    {
        ui->statusDisplay->append(tr("Failed. Make sure no other application is using the disk."));
    }
    ui->statusDisplay->append(tr("Applying drafted partition table..."));
    std::unique_ptr<GPTDisk>& tgtDisk = disks[selectedGPTDiskIdx];
    tgtDisk->writeRaw(0, 512 + 128*128, draftedTable.get());
    tgtDisk->writeRaw(tgtDisk->getSize() - 512, 512, draftedTable.get());
    tgtDisk->writeRaw(tgtDisk->getSize() - 512 - 128 * 128, 128 * 128, draftedTable.get());
    ui->statusDisplay->append(tr("All Done."));
}

void MainWindow::copyImageProgress(uint64_t copied)
{
    // scale the progress according to max.
    ui->copyProgressBar->setMaximum(MAX_PROGRESS);
    ui->copyProgressBar->setValue(MAX_PROGRESS * (double)copied / (double)isoDisk->getSize()+1) ;
    uint64_t currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    // progress report is per 1MB
    double bytesPerMs = 1024.0 * 1024 * 1000 / (currentTime - lastProgressReportTime);
    ui->speedLabel->setText(QString("%2:%1/s").arg(sizeToHumanReadable(bytesPerMs)).arg(sizeToHumanReadable(copied)));
    lastProgressReportTime = currentTime;

}

void MainWindow::discoverDrives()
{
    // Fetch all available drives.
    int diskNr = -1;
    int reachedEnd = false;
    disks.clear();
    invalidDisks.clear();

    while (!reachedEnd)
    {
        diskNr++;
        QString diskName = QString("\\\\.\\PhysicalDrive%1").arg(diskNr);
        std::unique_ptr<GPTDisk> disk;
        GPTCreateErrorCode result = GPTDisk::createGPT(diskName, &disk);
        switch (result)
        {
            case GPTCreateErrorCode::ERROR_NONE:
                qInfo() << "GPT Disk discovered: " << diskName;
                disks.push_back(std::move(disk));
                break;
            case GPTCreateErrorCode::ERROR_OPENING:
                reachedEnd = true;
                break;
            case GPTCreateErrorCode::ERROR_HEADER_INTEGRITY:
                invalidDisks[diskName] = tr("This GPT disk seems to have broken headers. Repair it first.");
                break;
            case GPTCreateErrorCode::ERROR_NOT_GPT:
                invalidDisks[diskName] = tr("This is not a GPT disk.\nThis tool only supports GPT disks.\n"
                    "Please use utilities such as MBR2GPT.EXE(and diskpart) or gparted to convert it to GPT.");
                break;
            default:
                invalidDisks[diskName] = tr("Unknown error.");
                break;
        }
    }
}






void MainWindow::on_stopButton_clicked()
{
    worker.stopCopy();
}


void MainWindow::on_settingsButton_clicked()
{
    mySettingsDialog.show();
}

