#include "settingsdialog.h"
#include "qdebug.h"
#include "qtranslator.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->languageComboBox->addItem(QString("en_US English"));
    ui->languageComboBox->addItem(QString("zh_CN 简体中文"));

}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
    QString selectedLocaleStr = ui->languageComboBox->currentText();
    QStringRef localeStr(&selectedLocaleStr, 0, 5);
    // QLocale locale = localeStr.toString();


    QString localePath = QString(":/translate/Sear_") + localeStr.toString() + ".qm";
    bool result = trans.load(localePath);
    qDebug() << "Translate to " << localePath;
    if (!result) qDebug() << "Failed";


    QCoreApplication::installTranslator(&trans);
}

