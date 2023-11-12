#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "qtranslator.h"
#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::SettingsDialog *ui;
    QTranslator trans;
};

#endif // SETTINGSDIALOG_H
