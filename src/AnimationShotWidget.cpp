#include "AnimationShotWidget.h"
#include "OptionWindow.h"
#include <qDebug>
#include <KoiAnimation.h>
#include <QCoreApplication>
#include <SalmonActions.h>
#include <GoatScreenshot.h>
#include <QThread>
#include <QDir>
#include <QSettings>
#include <OwlKeyLogLabel.h>
#include <OwlMouseLogLabel.h>
#include <OwlKeyHook.h>
#include <DeerShortcut.h>
#include <QMessageBox>
#include <QFileDialog>
#include <Utils.h>
#include <QLabel>
#include <qDebug>
#include <QSystemTrayIcon>
#include <QMouseEvent>
#include <QCloseEvent>
#include <KoiMovie.h>
#include <AniImageProcess.h>
#include <QLinearGradient>
#include <KoiConvert.h>
QSettings *GlobalOption;
AnimationShotWidget::AnimationShotWidget(const QRect geo, QWidget* parent) 
    :DynamicShotWidget(geo, parent)
{
    connect(&mTimer, &QTimer::timeout, this, &AnimationShotWidget::onTimeout);
    qInfo() << "AnimationShotWidget init on thread:" << QThread::currentThreadId();
    mAnimation = new KoiAnimation();
    mKeyHook = new OwlKeyHook();
    mKeyLogLabel = new OwlKeyLogLabel(mKeyHook, this);
    mHandleTread = new QThread();
    mMouseLogLabel = new OwlMouseLogLabel(mKeyHook, mShotRect);
    mKeyHook->moveToThread(mHandleTread);
    mHandleTread->start();
    mKeyLogLabel->move(10, this->height() - 10 - mKeyLogLabel->height());
    mKeyLogLabel->setVisible(false);
    mPlayedTime = 0;
    connect(this, &DynamicShotWidget::SigGeometryChanged, 
        this, &AnimationShotWidget::limitKeyLogRange);
        
    connect(this, &DynamicShotWidget::SigGeometryChanged, 
        [&](){this->updateInformation();} );
    this->actionBarInit();
    this->hotkeyInit();
}

AnimationShotWidget::~AnimationShotWidget()
{
    mTimer.stop();
    qInfo() << ("AnimationShotWidget uninit");
    if (mActBar)mActBar->deleteLater();
    if (mInforBar)mInforBar->deleteLater();
    if (mMouseLogLabel) mMouseLogLabel->deleteLater();
    if (mHandleTread){
        mHandleTread->quit();
        mHandleTread->deleteLater();
    }
    if (mKeyHook) mKeyHook->deleteLater();
    mAnimation->deleteLater();
    if(mScreenShot) delete mScreenShot;
    if(Opt_Bool(_opt_AutoCleanFile)){
        if(Opt_Bool(_opt_OnlyCleanOvertimeFile)){
            qInfo() << "remove history file over " << Opt_Int(_opt_CleanFileOverdays) << " days";
            clearFiles(Opt_Str(_opt_HistoryFilePath), Opt_Int(_opt_CleanFileOverdays), mDelExclude);
        }else{
            qInfo() << ("remove all history file.");
            clearFiles(Opt_Str(_opt_HistoryFilePath), 0, mDelExclude);
        }
    }
}
static bool hotkeyReg(QString optKey, DeerShortcut **shortPtr)
{
    *shortPtr = GlobalOption->findChild<DeerShortcut *>(optKey);
    if(!*shortPtr) {
        *shortPtr = new DeerShortcut(GlobalOption);
        (*shortPtr)->setObjectName(optKey);
    }
    qInfo() << "Registe hotkey " << Opt_Str(optKey) << " for " << optKey;
    if(Opt_Str(optKey).isEmpty()) return true;
    if((*shortPtr)->setShortcut(Opt_KeySeq(optKey))){
        return true;
    } else {
        qInfo() << "Fail registe hotkey " << Opt_Str(optKey) << " for " << optKey;
        return false;
    }
}

void AnimationShotWidget::hotkeyInit()
{
    QString ErrorString;
    if(!hotkeyReg(_opt_HotkeyRecord, &mShortcutRecord)){
        ErrorString += QString("Registration record hotkey %1 failed.\n")
            .arg(Opt_Str(_opt_HotkeyRecord));
    }
    connect(mShortcutRecord, &DeerShortcut::activated,
        this, &AnimationShotWidget::recordAnimition);
    
    if(!hotkeyReg(_opt_HotkeyPause, &mShortcutPause)){
        ErrorString += QString("Registration pause/continue hotkey %1 failed.\n")
            .arg(Opt_Str(_opt_HotkeyPause));
    }
    connect(mShortcutPause, &DeerShortcut::activated,
        this, &AnimationShotWidget::pauseAnimition);

    if(!ErrorString.isEmpty()){
        QMessageBox::warning(this,"Error", ErrorString);
    }
}

QString AnimationShotWidget::generateFilepath(int format)
{
    QDir saveDir(Opt_Str(_opt_HistoryFilePath));
    QString filename = generateFilename(format);
    return saveDir.filePath(filename);
}

QString AnimationShotWidget::generateFilename(int format)
{
    QString timeFormat = Opt_Str(_opt_FileTimeFormat);
    if(format < 0) Opt_Int(_opt_RecordFormat);
    QString filename = QDateTime::currentDateTime().toString(timeFormat);
    filename += "." + KoiAnimation::FormatStr(format);
    return filename;
}

void AnimationShotWidget::changeStatus(StatusCode status)
{
    switch (mStatus)
    {
    case StatusCode::AnimationPlaying:
        this->statusAnimationPlayingSet(false);
        break;
    case StatusCode::AnimationPreview:
        this->statusAnimationPreviewSet(false);
        break;
    case StatusCode::PrepareRecord:
        this->statusPrepareRecordSet(false);
        break;
    case StatusCode::Recording:
        this->statusRecordingSet(false);
        break;
    case StatusCode::Converting:
        this->statusConvertingSet(false);
        break;
    default:
        break;
    }
    mStatus = status;
    switch (mStatus)
    {
    case StatusCode::AnimationPlaying:
        this->statusAnimationPlayingSet(true);
        break;
    case StatusCode::AnimationPreview:
        this->statusAnimationPreviewSet(true);
        break;
    case StatusCode::PrepareRecord:
        this->statusPrepareRecordSet(true);
        break;
    case StatusCode::Recording:
        this->statusRecordingSet(true);
        break;
    case StatusCode::Converting:
        this->statusConvertingSet(true);
        break;
    default:
        break;
    }
    updateInformation();
}
void AnimationShotWidget::statusPrepareRecordSet(bool enable)
{
    qInfo() << "Status PrepareRecord turn to " << enable;
    if(enable){
        mBtnRecord->setIcon(SalActIcon::Record());
        mPlayedTime = 0;
        mRecordFPS = 0;
        this->staysOnTop(true);
        this->showTaskIcon(false);
    }
}
void AnimationShotWidget::statusRecordingSet(bool enable)
{
    qInfo() << "Status Recording turn to " << enable;
    mLockGeo = enable;
    if(mScreenShot) delete mScreenShot;
    mScreenShot = GoatScreenshot::Create(
        mShotRect, 
        GoatScreenshot::getScreenByWidget(this),
        true
    );
    mBtnPause->setEnabled(enable);
    if(mBtnKeyLog->isChecked()){
        mKeyHook->setKeyHookEnable(enable);
        mKeyHook->setMouseHookEnable(enable);
        mKeyLogLabel->setVisible(enable);
    }
    mBtnKeyLog->blockSignals(!enable);
    if (enable) {
        mStartTime = QDateTime::currentMSecsSinceEpoch();
        mRecodeTimeInv = 1000 / Opt_Int(_opt_MaxFPS);
        mTimer.start(1000 / Opt_Int(_opt_MaxFPS));
        mLastUpdateInfoTime = 0;
        mIsAddedProgress = false;
        mLostFrameStats = {0};
        mBtnRecord->setIcon(SalActIcon::StopRecord());
        this->changeBoardColor(mRecordColor);
        KoiFormat format = CheckIsEncodeLater() ? KoiFormat::Dga : (KoiFormat)Opt_Int(_opt_RecordFormat);
        mAnimation->init(generateFilepath(format), format);
        qInfo() << "Recoding image to " << mAnimation->fileName() << "timer inv:" << mRecodeTimeInv;
        mRecordFPS = 0;
        this->staysOnTop(true);
        this->showTaskIcon(false);
    }
    else
    {
        if(mPausetime){
            pauseAnimition();
        }
        mTimer.stop();
        mAnimation->waitForIdle();
        mAnimation->finish(QDateTime::currentMSecsSinceEpoch() - mStartTime);
        mAnimation->waitForIdle();
        this->changeBoardColor(QColor());
        mRecordFPS = (float)mAnimation->frameCount() * 1000.0 / mAnimation->timeStamp();
        qInfo() << "stop recoding. filesize:" << FileSizeString(mAnimation->encodedSize()) << 
           ", frame count:" << mAnimation->frameCount() << 
           ", time:" << mAnimation->timeStamp() << 
           ", fps:" << mRecordFPS;
        qInfo() << "Frame lost: capture:" << mLostFrameStats.capture <<
        ", encode:" << mLostFrameStats.encode;
        delete mScreenShot;
        mScreenShot = nullptr;
    }
    for (QWidget* wid : mEnbOnFinishList) wid->setEnabled(!enable);
    this->update();
}
void AnimationShotWidget::statusConvertingSet(bool enable)
{
    static QString filenameCache;
    const int ringSize = 150;
    if(enable){
        this->showBorder(false);
        this->repaint();
        this->showInformation(false);
        mBtnRecord->setIcon(SalActIcon::Return());
        mGeoSave = this->geometry();
        auto newRect = QRect(
            mGeoSave.left() + (mGeoSave.width() - ringSize) / 2,
            mGeoSave.top() + (mGeoSave.height() - ringSize) / 2,
            ringSize, ringSize);
        this->setGeometry(newRect);
        this->onGeometryChanged();
        mPlayProgress = new PlayProgress(this);
        mPlayProgress->setGeometry(0, 0, ringSize, ringSize);
        mPlayProgress->setStyle(PlayProgress::Ring);
        mPlayProgress->show();
        QList<QWidget *> *actList = mActBar->actionList();
        for (QWidget *wid : *actList)
        {
            if(wid == mBtnRecord 
                || wid == mBtnClose
                || wid == mLblTime
                || wid == mMoveControl){
                continue;
            }
            wid->setVisible(false);
        }
        mLblTime->setFixedWidth(mLblTime->fontMetrics().width(tr("Encoding...")));
        mLblTime->setText(tr("Encoding..."));
        mActBar->readjustWidth();
        mActBar->reAdjustGeo();
        int dur = mAnimation->timeStamp();
        filenameCache = mAnimation->fileName();
        mConvert = new KoiConvert(mAnimation->fileName(), 
            (KoiFormat)Opt_Int(_opt_RecordFormat), mAnimation);
        mConvert->setDuration(dur);
        if(Opt_Bool(_opt_AddProgressBar)){
            mConvert->addProgressBar(true);
            mIsAddedProgress = true;
        }
        connect(mConvert, &KoiConvert::sigFrameConverted, 
            this, &AnimationShotWidget::onFrameConverted);
        mConvert->encodeNextFrame();
        this->staysOnTop(true);
        this->showTaskIcon(false);
    }else{
        delete mConvert;
        mConvert = nullptr;
        delete mPlayProgress;
        mPlayProgress = nullptr;
        this->setGeometry(mGeoSave);
        this->repaint();
        this->showBorder(true);
        for (QWidget *wid : *mActBar->actionList())
        {
            wid->setVisible(true);
        }
        mLblTime->setFixedWidth(mLblTime->fontMetrics().width(tr("00:00")) + 10);
        mLblTime->setText(tr("00:00"));
        mActBar->readjustWidth();
        mActBar->reAdjustGeo();
        this->onGeometryChanged();
        if(mBtnInfo->isChecked()){
            this->showInformation(true);
        }
        qInfo() << "delete cache file " << filenameCache << 
        QFile(filenameCache).remove();
    }
}

void AnimationShotWidget::onFrameConverted()
{
    mPlayProgress->setValue(mConvert->progress()*100);
    if(mConvert->isFinish())
    {
        changeStatus(StatusCode::AnimationPreview);
    }else{
        mConvert->encodeNextFrame();
    }
}

void AnimationShotWidget::statusAnimationPreviewSet(bool enable)
{
    qInfo() << "Status Preview turn to " << enable;
    mLockGeo = enable;
    if(enable){
        mBtnRecord->setIcon(SalActIcon::Return());
        KoiMovie tMovie;
        tMovie.setFileName(mAnimation->fileName());
        tMovie.jumpToFrame(0);
        mShowFrame = tMovie.currentImage();
        QPainter painter(&mShowFrame);
        QBrush brush(QColor(0, 0, 0, 128));
        QRect drawPicRect(QPoint(0, 0), mShowFrame.size());
        painter.fillRect(drawPicRect, brush);
        QPixmap finishImg(":/icon/play.png");
        painter.drawPixmap(
            QPoint((mShowFrame.width() - finishImg.width()) / 2, 
                    (mShowFrame.height() - finishImg.height()) / 2)
            , finishImg);
        mShowImagePtr = &mShowFrame;
        this->staysOnTop(false);
        this->showTaskIcon(true);
    }else{
        mShowFrame = QImage();
        mShowImagePtr = nullptr;
    }
    this->update();
}
void AnimationShotWidget::onMoiveFrameChanged(int frameNumber)
{
    mShowFrame = mMovie->currentImage();
    mShowImagePtr = &mShowFrame;
    this->update();
}

void AnimationShotWidget::onProgressMousePress(QMouseEvent *event)
{
    mNeedPlay = !(mMovie->state() == KoiMovie::Paused);
    if(mNeedPlay){
        pauseAnimition();
    }
}

void AnimationShotWidget::onProgressMouseRelease(QMouseEvent *event)
{
    if(mNeedPlay){
        pauseAnimition();
    }
}

void AnimationShotWidget::onProgressValChangeRequest(int valRequest)
{
    if(valRequest >= mAnimation->timeStamp()) valRequest = mAnimation->timeStamp();
    if(valRequest < 0) valRequest = 0;
    mMovie->jumpToTimeStamp(valRequest);
}

void AnimationShotWidget::statusAnimationPlayingSet(bool enable)
{
    qInfo() << "Status Playing turn to " << enable;
    mLockGeo = enable;
    mBtnPause->setEnabled(enable);
    if(enable){
        this->changeBoardColor(mPlayingColor);
        if(mMovie) delete mMovie;
        mMovie = new KoiMovie(this);
        connect(mMovie, &KoiMovie::frameChanged, this,  &AnimationShotWidget::onMoiveFrameChanged);
        if(mAnimation->format() == KoiFormat::APNG){
            mMovie->setFormat("apng");
        }
        mMovie->setFileName(mAnimation->fileName());

        mPlayProgress = new PlayProgress(this);
        mPlayProgress->setMaxValue(mAnimation->timeStamp());
        mPlayProgress->setGeometry(
            mDrawRect.left() + mPenWidth / 2, 
            mDrawRect.bottom() - mPenWidth / 2 + 1,
            mDrawRect.width() - mPenWidth,
            mPenWidth);
        mPlayProgress->show();
        if(mAnimation->format() == KoiFormat::WEBP && mMovie->frameCount() == 1) {
            mPlayProgress->setValue(mAnimation->timeStamp());
            mMovie->jumpToFrame(0);
            mShowFrame = mMovie->currentImage();
            mShowImagePtr = &mShowFrame;
        } else {
            connect(mPlayProgress, &PlayProgress::sigValChangeRequest, 
                this, &AnimationShotWidget::onProgressValChangeRequest);
            connect(mPlayProgress, &PlayProgress::sigValMousePress, 
                this, &AnimationShotWidget::onProgressMousePress);
            connect(mPlayProgress, &PlayProgress::sigValMouseRelease, 
                this, &AnimationShotWidget::onProgressMouseRelease);
            mTimer.start(20);
            mMovie->start();
        }
        qInfo() << ("mMovie start");
        this->staysOnTop(false);
        this->showTaskIcon(true);
    }else{
        if(mPausetime){
            pauseAnimition();
        }
        this->changeBoardColor(QColor());
        mMovie->stop();
        mTimer.stop();
        delete mMovie;
        mMovie = nullptr;
        delete mPlayProgress;
        mPlayProgress = nullptr;
        mShowImagePtr = nullptr;
    }
    mBtnPause->setIcon(SalActIcon::Pause());
    this->update();
}

void AnimationShotWidget::limitKeyLogRange()
{
    mKeyLogLabel->move(10, this->height() - 10 - mKeyLogLabel->height());
    mMouseLogLabel->setLimitRect(mShotRect);
    // qInfo() << "Geometry change to " << this->geometry() << ", shot rect " << mShotRect;
}
void AnimationShotWidget::setOptionSetting(QSettings* setting)
{
    GlobalOption = setting;
    /*=======default options========*/
    Opt_SetDef(_opt_RecordFormat, KoiFormat::GIF);
    Opt_SetDef(_opt_HistoryFilePath, QCoreApplication::applicationDirPath() + "/HistoryFile");
    Opt_SetDef(_opt_FileTimeFormat, "dd-MM-yyyy-hh-mm-ss");
    Opt_SetDef(_opt_AutoCleanFile, 1);
    Opt_SetDef(_opt_OnlyCleanOvertimeFile, 1);
    Opt_SetDef(_opt_CleanFileOverdays, 7);
    Opt_SetDef(_opt_CloseAfterCopy, 1);
    Opt_SetDef(_opt_CloseAfterSave, 1);
    Opt_SetDef(_opt_MaxFPS, 20);
    Opt_SetDef(_opt_EncoderLater, 1);

    Opt_SetDef(_opt_HotkeyRecord, "F6");
    Opt_SetDef(_opt_HotkeyPause, "F7");
    Opt_SetDef(_opt_AddProgressBar, 0);
    
    QDir saveDir(Opt_Str(_opt_HistoryFilePath));
    if(!saveDir.exists()){
        saveDir.mkpath(Opt_Str(_opt_HistoryFilePath));
    }
    
    /*==============================*/
    KoiAnimation::setOptionSetting(GlobalOption);
    OptionWindow::setOptionSetting(GlobalOption);
}

void AnimationShotWidget::addProgressBar(QString srcName, QString dstName, KoiFormat format)
{
    KoiMovie tMovie;
    tMovie.setFileName(srcName);
    KoiAnimation tAnimation;
    QImage img;
    tAnimation.init(dstName, format);
    int ts = 0;
    tMovie.jumpToFrame(0);
    do
    {
        img = AddProgressBar(tMovie.currentImage(), tMovie.timeStamp(), tMovie.duration());
        tAnimation.addFrameTimestamp(img, tMovie.timeStamp());
        tAnimation.waitForIdle();
        ts = tMovie.timeStamp();
        while (ts + 30 < tMovie.nextTimeStamp() - 20)
        {
            ts += 30;
            img = AddProgressBar(tMovie.currentImage(), ts, tMovie.duration());
            tAnimation.addFrameTimestamp(img, ts);
            tAnimation.waitForIdle();
        }
        tMovie.jumpToNextFrame();
    } while (tMovie.currentFrameNumber() != 0);
    tAnimation.finish(tMovie.duration());
    tAnimation.waitForIdle();
}

void AnimationShotWidget::pauseAnimition()
{
    if(mStatus == Recording){
        if(mPausetime){
            qInfo() << ("Continue recoder.");
            mStartTime = QDateTime::currentMSecsSinceEpoch() - mPausetime;
            mPausetime = 0;
            this->changeBoardColor(mRecordColor);
            mBtnPause->setIcon(SalActIcon::Pause());
        }else{
            qInfo() << ("Pause recoder.");
            mPausetime = QDateTime::currentMSecsSinceEpoch() - mStartTime;
            this->changeBoardColor(mPauseColor);
            mBtnPause->setIcon(SalActIcon::Play());
        }
    }else if(mStatus == AnimationPlaying){
        if(mAnimation->format() == KoiFormat::WEBP && mMovie->frameCount() == 1) {
            return;
        }
        if(mMovie->state() == KoiMovie::Paused){
            mPausetime = 0;
            mMovie->setPaused(false);
            mBtnPause->setIcon(SalActIcon::Pause());
            this->changeBoardColor(mPlayingColor);
        } else {
            mMovie->setPaused(true);
            mBtnPause->setIcon(SalActIcon::Play());
            this->changeBoardColor(mPauseColor);
        }
    }
}
void AnimationShotWidget::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
void AnimationShotWidget::onMousePress(const QPoint& pot, int zoomflag)
{
    if(zoomflag == 0x22){
        switch (mStatus)
        {
        case StatusCode::AnimationPreview:
            changeStatus(StatusCode::AnimationPlaying);
            break;
        case StatusCode::AnimationPlaying:
        {
            pauseAnimition();
            break;
        }
        default:
            break;
        }
    }
}
void AnimationShotWidget::recordAnimition()
{
    switch (mStatus)
    {
    case StatusCode::AnimationPreview:
    case StatusCode::AnimationPlaying:
        changeStatus(StatusCode::PrepareRecord);
        break;
    case StatusCode::PrepareRecord:
        changeStatus(StatusCode::Recording);
        break;
    case StatusCode::Recording:
        if(CheckIsEncodeLater()){
            changeStatus(StatusCode::Converting);
        } else {
            changeStatus(StatusCode::AnimationPreview);
        }
        break;
    case StatusCode::Converting:
        changeStatus(StatusCode::PrepareRecord);
        break;
    default:
        break;
    }
}

void AnimationShotWidget::updateInformation()
{
    if(mStatus != Converting){
        mLblTime->setText(QString("%1:%2")
            .arg(mPlayedTime / 1000 / 60, 2, 10, QLatin1Char('0'))
            .arg((mPlayedTime / 1000) % 60, 2, 10, QLatin1Char('0')));
    }
    if(mInforBar){
        mInforRectSize->setText(SizeString(mShotRect.size()));
        if(mAnimation->encodedSize() == 0){
            mInfoFileSize->setText("-");
        }else{
            mInfoFileSize->setText(FileSizeString(mAnimation->encodedSize()));
        }
        mInfoFPS->setText(QString("FPS:%1")
            .arg(QString::number(mRecordFPS <= 0?mAnimation->framePerSec():mRecordFPS,
             'f', 1)));
    }
}

void AnimationShotWidget::refreshInfoFormatIcon()
{
    mBtnFormat->setIcon(FormatIcon());
}

void AnimationShotWidget::showInformation(bool enable)
{
    if (enable) {
        mInforBar = new ActionsBar();
        mBtnSetting = mInforBar->addButton(SalActIcon::Setting());
        connect(mBtnSetting, &QPushButton::clicked, [&](){
            OptionWindow *optWin = new OptionWindow();
            connect(optWin, &OptionWindow::sigFormatChanged
            , this, &AnimationShotWidget::refreshInfoFormatIcon);
            this->hide();
            this->mActBar->hide();
            if(this->mInforBar)this->mInforBar->hide();
            optWin->setModal(true);
            optWin->exec();
            delete optWin;
            this->show();
            mActBar->show();
            if(this->mInforBar)this->mInforBar->show();
        });
        mEnbOnFinishList.append(mBtnSetting);
        mBtnSetting->setEnabled(!mTimer.isActive());

        mBtnFormat = mInforBar->addButton(FormatIcon());
        mBtnFormat->setIconSize(QSize(32, 32));
        connect(mBtnFormat, &QPushButton::clicked, this, [&](){
            int val = (Opt_Int(_opt_RecordFormat) + 1) % (Format_Count - 1);
            Opt_SetVal(_opt_RecordFormat, val);
            qInfo() << "Set format to " << KoiAnimation::FormatStr(Opt_Int(_opt_RecordFormat));
            refreshInfoFormatIcon();
        });
        mEnbOnFinishList.append(mBtnFormat); 
        mBtnFormat->setEnabled(!mTimer.isActive());

        mInforRectSize = mInforBar->addLabel("9999X9999");
        mInforBar->addGapLine();
        mInfoFPS = mInforBar->addLabel("FPS:00.0");
        mInforBar->addGapLine();
        mInfoFileSize = mInforBar->addLabel("0000 kb");
        
        this->updateInformation();
        mActBar->addLowWidget(mInforBar);
        mInforBar->show();
    }
    else
    {
        mEnbOnFinishList.removeOne(mBtnSetting);
        mEnbOnFinishList.removeOne(mBtnFormat);
        mActBar->deleteLowWidget(mInforBar);
        mInforBar = nullptr;
    }
}

void AnimationShotWidget::onTimeout()
{
    if(mStatus == StatusCode::Recording){
        if(mPausetime)return;
        qint64 tmpPlayedTime = mPlayedTime;
        mPlayedTime = QDateTime::currentMSecsSinceEpoch() - mStartTime;
        if(mPlayedTime - tmpPlayedTime > mRecodeTimeInv + 10){
            mLostFrameStats.capture++;
        }
        qint64 cost = QDateTime::currentMSecsSinceEpoch();
        mScreenShot->changeShotRect(mShotRect);
        mScreenShot->getNextFrame();
        QImage frame = mScreenShot->screenImageWithMouse();
        if(mAnimation->isBusy()) {
            mLostFrameStats.encode++;
        }else{
            mAnimation->addFrameTimestamp(frame, mPlayedTime);
        }
        cost = QDateTime::currentMSecsSinceEpoch() - cost;
        mTimer.setInterval(mRecodeTimeInv - cost % mRecodeTimeInv);
        if(mPlayedTime - mLastUpdateInfoTime > 1000){
            this->updateInformation();
            mLastUpdateInfoTime = mPlayedTime;
        }

    }else if(mStatus == StatusCode::AnimationPlaying){
        mPlayedTime = mMovie->timeStamp();
        mPlayProgress->setValue(mPlayedTime);
        this->updateInformation();
    }
}

QImage AnimationShotWidget::shotOnce()
{
    QScreen *scr = GoatScreenshot::getScreenByWidget(this);
    return GoatScreenshot::quickShot(mShotRect, scr, true);
}

void AnimationShotWidget::onCopyAction()
{
    if(mAnimation->fileName().isEmpty()){
        QImage frame = shotOnce();
        qInfo() << "Copy static image " << frame << " with " << mShotRect;
        ClipSetImage(frame);
    }else{
        if (mStatus == Recording)return;
        QString cFilename;
        cFilename = mAnimation->fileName();
        if(Opt_Bool(_opt_AddProgressBar) && !mIsAddedProgress){
            cFilename = addFilenameSuffix(cFilename, "_progressbar");
            addProgressBar(mAnimation->fileName(), cFilename, mAnimation->format());
        }
        qInfo() << "Copy image file " << cFilename << ", ret : " << ClipSetFile(cFilename);
        mDelExclude = cFilename;
    }
    if(Opt_Bool(_opt_CloseAfterCopy)){
        this->deleteLater();
    }
}

void AnimationShotWidget::onTrayIconStateChange(bool isHideToTray)
{
    qInfo() << "isHideToTray: " << isHideToTray;
    static bool isShowTipsFlag = false;
    if (isHideToTray && !isShowTipsFlag)
    {
        isShowTipsFlag = true;
        QSystemTrayIcon *trayIcon = mActBar->systemTrayIcon();
        trayIcon->showMessage(tr("Dogif"), tr("Dogif is hidden in the tray, right click on the Dogif icon to show the control menu bar"));
    }
}

void AnimationShotWidget::onSaveAction()
{
    QString filter;
    QImage img;
    switch (Opt_Int("OptRecordFormat"))
    {
    case KoiFormat::GIF:
        filter = tr("Graphics Interchange Format(*.gif)");
        break;
    case KoiFormat::APNG:
        filter = tr("Animated Portable Network Graphics(*.png)");
        break;
    case KoiFormat::WEBP:
        filter = tr("Animated WebP(*.webp)");
        break;
    default:
        break;
    }
    if(mAnimation->fileName().isEmpty()){
        img = shotOnce();
        filter = tr("Portable Network Graphics(*.png);;Windows Bitmap (*.bmp);;Joint Photographic Experts Group (*.jpg *.jpeg)");
    }
    QString dirpath = QFileDialog::getSaveFileName(this, tr("Save Image"), 
        "", filter);
    if(dirpath.isEmpty())return;
    
    if(mAnimation->fileName().isEmpty()){
        qInfo() << "Save static image " << img << " with " << mShotRect;
        img.save(dirpath);
    }else{
        QFile::remove(dirpath);
        qInfo() << "Copy image file "<<mAnimation->fileName()<<" to " << dirpath;
        if(Opt_Bool(_opt_AddProgressBar) && !mIsAddedProgress){
            addProgressBar(mAnimation->fileName(), dirpath, mAnimation->format());
        } else {
            QFile::copy(mAnimation->fileName(), dirpath);
        }
    }
    if(Opt_Bool(_opt_CloseAfterSave)){
        this->deleteLater();
    }
}
void AnimationShotWidget::close()
{
    this->hide();
    if(mActBar) mActBar->hide();
    if(mInforBar) mInforBar->hide();
    this->deleteLater();
}
void AnimationShotWidget::actionBarInit()
{
    mActBar = new ActionsBar();
    
    connect(mActBar, &ActionsBar::SigTrayIconStateChange,
        this, &AnimationShotWidget::onTrayIconStateChange);

    mActBar->setTrayIcon(QIcon(":/icon/Dogif.png"));
    if(Opt_Int(_opt_SecondaryMode)){
        auto lbl = mActBar->addLabel(tr("Secondary"));
        lbl->setStyleSheet("color: rgb(255, 0, 0);");
    }
    mBtnInfo = mActBar->addButton(SalActIcon::Info());
    mBtnInfo->setCheckable(true);
    connect(mBtnInfo, &QPushButton::clicked, this, &AnimationShotWidget::showInformation);

    mLblTime = mActBar->addLabel("00:00", 60);
    mLblTime->setStyleSheet("font: 12pt \"Microsoft YaHei UI\";\ncolor: rgb(48, 48, 48);");
    mLblTime->setFixedWidth(mLblTime->fontMetrics().width(tr("00:00")) + 10);
    mMoveControl = mActBar->addMoveControl(this);
    connect(mActBar, &ActionsBar::sigMove, this, &DynamicShotWidget::move);
    connect(mMoveControl, &MoveControl::sigDoubleClick, this, &DynamicShotWidget::fitsWindow);

    mBtnKeyLog = mActBar->addButton(SalActIcon::Keyboard());
    mBtnKeyLog->setCheckable(true);
    mBtnKeyLog->blockSignals(true);
    connect(mBtnKeyLog, &QPushButton::clicked, mKeyHook, &OwlKeyHook::setKeyHookEnable);
    connect(mBtnKeyLog, &QPushButton::clicked, mKeyHook, &OwlKeyHook::setMouseHookEnable);
    connect(mBtnKeyLog, &QPushButton::clicked, mKeyLogLabel, &QWidget::setVisible);

    mBtnRecord = mActBar->addButton(SalActIcon::Record());
    connect(mBtnRecord, &QPushButton::clicked, this, &AnimationShotWidget::recordAnimition);

    mBtnPause = mActBar->addButton(SalActIcon::Pause());
    mBtnPause->setEnabled(false);
    connect(mBtnPause, &QPushButton::clicked, this, &AnimationShotWidget::pauseAnimition);

    QPushButton* act = mActBar->addButton(SalActIcon::Location());
    connect(act, &QPushButton::clicked, [&]() {
        qInfo() << "Location file " << mAnimation->fileName();
        OpenFolderAndSelectFile(mAnimation->fileName());
        });
    act->setEnabled(false);
    mEnbOnFinishList.append(act); 
    
    act = mActBar->addButton(SalActIcon::Copy());
    connect(act, &QPushButton::clicked, this, &AnimationShotWidget::onCopyAction);

    act = mActBar->addButton(SalActIcon::Save());
    connect(act, &QPushButton::clicked, this, &AnimationShotWidget::onSaveAction);

    mBtnClose = mActBar->addButton(QIcon(SalActIcon::Close()));
    connect(mBtnClose, &QPushButton::clicked, this, &AnimationShotWidget::close);

    mActBar->show();
    mActBar->followAdjust(this->geometry());
    connect(this, &DynamicShotWidget::SigGeometryChanged, 
        mActBar, &ActionsBar::followAdjust);

    connect(this, &AnimationShotWidget::SigStaysOnTopChanged,
        mActBar, &ActionsBar::staysOnTop);
    connect(this, &AnimationShotWidget::SigWindowActivate,[&](){
        if(mActBar->isVisible())mActBar->raise();
    });
    connect(mActBar, &ActionsBar::SigWindowActivate, [&](){
        this->raise();
        mActBar->raise();
    });

}

void AnimationShotWidget::paintEvent(QPaintEvent* event)
{
    DynamicShotWidget::paintEvent(event);
    QPainter painter(this);
    if (mShowImagePtr && !mShowImagePtr->isNull())
    {
        painter.drawImage(mShotRectOnDialog, *mShowImagePtr, 
            QRect( 0, 0, mShotRectOnDialog.width(), mShotRectOnDialog.height()));
    } 
}