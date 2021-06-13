#include <OptionWindow.h>
#include <QSettings>
#include <QDebug>
#include <QCheckbox>
#include <QComboBox>
#include <QSpinBox>
#include <QDateTime>
#include <QDir>
#include <QUrl>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QFileDialog>
#include <DeerShortcut.h>
#include <QDesktopServices>
#include <QApplication>
#include "KeySequenceEdit.h"
#include "AnimationShotWidget.h"
#include <RobinLog.h>
#include <fvupdater.h>
#include <QLocale>
#include <Utils.h>
#include <Version.h>

#include <ui_OptionWindow.h>
QSettings* mStaticOption = nullptr;

void HandleFunc(QObject *obj, QString optKey)
{
    QAbstractButton* absObj = qobject_cast<QAbstractButton *>(obj);
    if(absObj && absObj->isCheckable())
    {
        absObj->setChecked((bool)mStaticOption->value(optKey).toInt());
        return;
    }
    QAbstractSlider* sliObj = qobject_cast<QAbstractSlider *>(obj);
    if(sliObj)
    {
        sliObj->setValue(mStaticOption->value(optKey).toInt());
        return;
    }
    QComboBox* cmbObj = qobject_cast<QComboBox*>(obj);
    if(cmbObj){
        cmbObj->setCurrentIndex(mStaticOption->value(optKey).toInt());
        return;
    }
    QLineEdit* edtObj = qobject_cast<QLineEdit*>(obj);
    if(edtObj){
        KeySequenceEdit* shtObj = qobject_cast<KeySequenceEdit*>(obj);
        if(!shtObj){
            edtObj->setText(mStaticOption->value(optKey).toString());
        }
        return;
    }
    QSpinBox* spinObj = qobject_cast<QSpinBox*>(obj);
    if(spinObj){
        spinObj->setValue(mStaticOption->value(optKey).toInt());
        return;
    }
}
void TraverAllChild(QObject *obj)
{
    for (QObject *child : obj->children())
    {
        TraverAllChild(child);
    }
    QString optKey = obj->objectName().split("_").first();
    if(!optKey.isEmpty() && mStaticOption->contains(optKey)){
        obj->blockSignals(true);
        HandleFunc(obj, optKey);
        obj->blockSignals(false);
    };
}
OptionWindow::OptionWindow(QWidget *parent):QDialog(parent)
{
    ui = new Ui_OptionWindow();
    ui->setupUi(this);
    ui->tabWidget->tabBar()->setVisible(false);
    ui->tabWidget->setCurrentIndex(0);
    // this->setWindowFlag(Qt::WindowStaysOnTopHint);    
    mSaveOpacityEffect = new QGraphicsOpacityEffect(this);
    ui->lblSaved->setGraphicsEffect(mSaveOpacityEffect);
    mSaveOpacityEffect->setOpacity(0);
    mSaveOpacityAnimation1 = new QPropertyAnimation(mSaveOpacityEffect, "opacity", this);
    InitLanguageCombox();
    TraverAllChild(this);
    QFile licFile(":/doc/Licensing.html");
    licFile.open(QFile::ReadOnly);
    ui->txtLic->setHtml(licFile.readAll());
    licFile.close();

    licFile.setFileName(":/doc/Ablout.html");
    licFile.open(QFile::ReadOnly);
    ui->textAbout->setHtml(licFile.readAll());
    licFile.close();
    
    onLosslessChanged(ui->OptWebpLossless->isChecked());
    ui->lblVersion->setText(QString("Version:") + STR_Version);
}
OptionWindow::~OptionWindow()
{
    delete ui;
}
void OptionWindow::aboutQt()
{
    QApplication::aboutQt();
}
void OptionWindow::onTimeFormatChanged(QString text)
{
    onOptionChanged(text);
    ui->lblTimeFormatPre->setText(AnimationShotWidget::generateFilename());
}

void OptionWindow::onSaved()
{
    mSaveOpacityAnimation1->stop();
    mSaveOpacityEffect->setOpacity(1);
    mSaveOpacityAnimation1->setDuration(1500);
    mSaveOpacityAnimation1->setStartValue(1);
    mSaveOpacityAnimation1->setEasingCurve(QEasingCurve::InCubic);
    // mSaveOpacityAnimation1->setKeyValueAt(0.7, 0.5);
    mSaveOpacityAnimation1->setEndValue(0);
    mSaveOpacityAnimation1->start();
}

void OptionWindow::onOptionChanged(QString newVal)
{
    QObject *obj = this->sender();
    mStaticOption->setValue(obj->objectName(), newVal);
    LOG_INFO("Set option[{}] to {}.", obj->objectName(), newVal);
    onSaved();
}

void OptionWindow::onOptionChanged(int newVal)
{
    QObject *obj = this->sender();
    mStaticOption->setValue(obj->objectName(), newVal);
    LOG_INFO("Set option[{}] to {}.", obj->objectName(), newVal);
    onSaved();
    if(obj->objectName() == "OptRecordFormat"){
        emit sigFormatChanged();
    }
}

void OptionWindow::onOptionChanged(bool newVal)
{
    QObject *obj = this->sender();
    mStaticOption->setValue(obj->objectName(), (int)newVal);
    LOG_INFO("Set option[{}] to {}.", obj->objectName(), newVal);
    onSaved();
}
void OptionWindow::on_btnCheckUpdate_clicked()
{
    FvUpdater* updater = FvUpdater::sharedUpdater();
    updater->setSkipVersionAllowed(false);
    updater->CheckForUpdatesNotSilent();
}
void OptionWindow::on_btnOpenFloder_clicked()
{
    QDesktopServices::openUrl(QUrl(mStaticOption->value(_opt_HistoryFilePath).toString(), QUrl::TolerantMode));
}

void OptionWindow::setOptionSetting(QSettings* setting)
{
    mStaticOption = setting;
    KeySequenceEdit::setOptionSetting(setting);
    InitUiByLanguage();
}

void OptionWindow::onLosslessChanged(bool isLossless)
{
    if(isLossless){
        ui->lblCompression->setText(tr("Compression Level:"));
        ui->lblFastest->setText(tr("Fastest"));
        ui->lblSmallest->setText(tr("Smallest"));
    }else{
        ui->lblCompression->setText(tr("Quality factor:"));
        ui->lblFastest->setText(tr("Smallest"));
        ui->lblSmallest->setText(tr("Clearest"));
    }
}
void OptionWindow::onLanguageChanged(QString lang)
{
    QComboBox *cmbLanguage = ui->cmbLanguage;
    QString strLanguageFile = cmbLanguage->currentData().toString();
    mStaticOption->setValue(_opt_Language, strLanguageFile);
    InitUiByLanguage();
    this->ui->retranslateUi(this);
}

void OptionWindow::on_OptHistoryFilePath_clicked()
{
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Select Directory"), 
        mStaticOption->value(_opt_HistoryFilePath).toString(), 
        QFileDialog::ShowDirsOnly);
    if(dirpath.isEmpty())return;
    onOptionChanged(dirpath);
    ui->OptHistoryFilePath_2->setText(dirpath);
}

bool FindLocalByCode(QString code, QLocale &retLocal)
{
    QList<QLocale> allLocales = QLocale::matchingLocales(
        QLocale::AnyLanguage,
        QLocale::AnyScript,
        QLocale::AnyCountry);
    for(QLocale &local : allLocales)
    {
        if(code.compare(local.name(),Qt::CaseInsensitive) == 0)
        {
            retLocal = local;
            return true;
        }
    }
    return false;
}

void OptionWindow::InitLanguageCombox()
{
    QComboBox *cmbLanguage = ui->cmbLanguage;
    QDir dir("./lang");
    if(!dir.exists())
    {
        qWarning() << dir.path() << " not exist";
        return ;
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString suffix = fileInfo.suffix();
        QLocale local;
        if(FindLocalByCode(fileInfo.baseName(), local)){
            cmbLanguage->addItem(local.nativeLanguageName(), fileInfo.filePath());
            qInfo() << __FUNCTION__ << local.name() << ":" << fileInfo.filePath();
        }
    }
    int inx;
    if(!mStaticOption->contains(_opt_Language)){
        QLocale locale;
        inx = cmbLanguage->findText(locale.nativeLanguageName());
        qInfo() << "set language to system default: " << locale.name();
    }else{
        inx = cmbLanguage->findData(Opt_Str(_opt_Language));
        qInfo() << "set language to save value: " << Opt_Str(_opt_Language);
    }
    cmbLanguage->setCurrentIndex(inx >= 0 ? inx : 0);
}

void OptionWindow::InitUiByLanguage()
{
    static QTranslator mTranslator;
    QString strLanguageFile = Opt_Str(_opt_Language);
    if (strLanguageFile.isEmpty())
    {
        qApp->removeTranslator(&mTranslator);
        return;
    }

    if (QFile(strLanguageFile).exists())
    {
        qApp->removeTranslator(&mTranslator);
        qInfo() << "load " << strLanguageFile << 
        mTranslator.load(strLanguageFile);
        qApp->installTranslator(&mTranslator);
    }
    else
    {
        qInfo() << "authclient language file does not exists ...";
    }
}