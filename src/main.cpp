#include <QApplication>
#include <AnimationShotWidget.h>
#include <QSettings>
#include <QDebug>
#include <breakpadwarp.h>
#define QAPPLICATION_CLASS QApplication
#include <SingleApplication>
#include <QFile>
#include <QMessageBox>
#include <Utils.h>
#include <QSysInfo>
#include <QDesktopWidget>
#include <QScreen>
#include <DogifLog.h>
#include <fvupdater.h>
#include <Version.h>

bool CheckSecondaryMode(SingleApplication &a, QString &configName)
{
    if(!a.isSecondary()){
        return true;
    }
    QString tips = QObject::tr("There is already an Dogif running, "
                    "whether in secondary mode or not. "
                    "In secondary mode, no hotkeys will be set for this instance "
                    "and the changed setting options will not be saved. "
                    "(Selecting No will exit the instance)");
    QMessageBox:: StandardButton result = 
                    QMessageBox::information(NULL, QObject::tr("Secondary Mode")
                        , tips, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(result != QMessageBox::Yes){
        exit(-1);
        return false;
    }
    qInfo() << ("Enter secondary mode.");
    QString newConfigName = QDateTime::currentDateTime().toString("dd-MM-yyyy-hh-mm-ss") + "_tmp.ini";
    if(QFile(configName).exists()){
        QFile::copy(configName, newConfigName);
    }
    QSettings opt(newConfigName, QSettings::IniFormat);
    opt.setValue(_opt_HotkeyRecord, "");
    opt.setValue(_opt_HotkeyPause, "");
    opt.setValue(_opt_SecondaryMode, 1);
    configName = newConfigName;
    return true;
}
void InitLog() {
    qInfo() <<  QApplication::applicationName() << " " << QApplication::applicationVersion();
    qInfo() <<  "buildAbi: " << QSysInfo::buildAbi();
    qInfo() <<  "run on: " << QSysInfo::prettyProductName() << " ["
        << QSysInfo::kernelType() << " " 
        << QSysInfo::kernelVersion() << " "
        << QSysInfo::currentCpuArchitecture() << "]";
    QDesktopWidget *desktopWidget = QApplication::desktop();
    qInfo() << "Screen count: " << desktopWidget->numScreens();
    for (size_t i = 0; i < desktopWidget->numScreens(); i++)
    {   
        QScreen *srn = QApplication::screens().at(0);
        qreal dotsPerInch = (qreal)srn->logicalDotsPerInch();
        qInfo() <<  "    "
        << desktopWidget->screenGeometry(i) << " DPI(log):"
        << srn->logicalDotsPerInch() << " DPI(phy):" <<  srn->physicalDotsPerInch();
    }
}

void InitInfo(){
    QApplication::setApplicationName(STR_AppName);
    QApplication::setApplicationVersion(STR_Version);
    QApplication::setOrganizationName(STR_Corporation);
    QApplication::setOrganizationDomain(STR_WebUrl);
}

void CheckUpdate()
{
    FvUpdater::sharedUpdater()->SetFeedURL("https://raw.githubusercontent.com/knightjun/Dogif/main/Update/Appcast.xml");
    FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
}

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv, true);
    InstallQtLogToRobin("dogif.log");
    InitInfo();
    InitLog();
    qInfo() << ("Install Crash Handler");
    google_breakpad::InstallCrashHandler();
    QString configName = "options.ini";
    CheckSecondaryMode(a, configName);
    qInfo() << "set config on " << configName;
    QSettings *opt = new QSettings(configName, QSettings::IniFormat);
/*==========默认设置==========*/
    if(!opt->contains(_opt_keeplastpos)){
        opt->setValue(_opt_keeplastpos, 1);
    }
/*============================*/
    AnimationShotWidget::setOptionSetting(opt);
    FvUpdater::setOptionSetting(opt);
    CheckUpdate();
    Statistics();
    QRect pos;
    if(opt->value(_opt_keeplastpos).toBool()){
        pos = opt->value("lastposrect").toRect();
    }
    qInfo() <<  "Set " << (opt->value(_opt_keeplastpos).toBool() ? "last" : "init") << " position:" << pos;
    AnimationShotWidget *mainWin = new AnimationShotWidget(pos);
    mainWin->show();
    QApplication::connect(mainWin, &AnimationShotWidget::destroyed, [&](){
        opt->setValue("lastposrect", mainWin->mShotRect);
        QString configName = opt->fileName();
        bool SecondaryMode = opt->value(_opt_SecondaryMode).toInt();
        delete opt;
        if(SecondaryMode){
            qInfo() << ("Remove config file on secondary mode.");
            QFile::remove(configName);
        }
        qInfo() << "Exit normal, shot rect : " << mainWin->mShotRect;
        a.quit();
    });
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}