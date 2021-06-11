#pragma once
#include "DynamicShotWidget.h"
#include <PlayProgress.h>

enum KoiFormat;
class KoiAnimation;
class ActionsBar;
class QLabel;
class KoiMovie;
class QPushButton;
class MoveControl;
class GoatScreenshot;

class OwlKeyLogLabel;
class OwlMouseLogLabel;
class OwlKeyHook;
class QSettings;
class DeerShortcut;
class KoiConvert;
class AnimationShotWidget : public DynamicShotWidget
{
	Q_OBJECT
public:
	AnimationShotWidget(const QRect geo = QRect(), QWidget* parent = nullptr);
	virtual ~AnimationShotWidget();

	void recordAnimition();
	void pauseAnimition();
	void updateInformation();
	void showInformation(bool enable);
	void refreshInfoFormatIcon();
	static void setOptionSetting(QSettings* setting);
	static QString generateFilename(int format = -1);
	static QString generateFilepath(int format);
signals:
	void sigPasteImage(QString filePath, QRect showRect);
protected:
	void paintEvent(QPaintEvent* event);
    virtual void onMousePress(const QPoint& pot, int zoomflag) override;
	virtual void closeEvent(QCloseEvent *event);
private:
	enum StatusCode{
		PrepareRecord,
		Recording,
		Converting,
		AnimationPreview,
		AnimationPlaying
	};
	void changeStatus(StatusCode status);
	void statusPrepareRecordSet(bool enable);
	void statusRecordingSet(bool enable);
	void statusConvertingSet(bool enable);
	void statusAnimationPreviewSet(bool enable);
	void statusAnimationPlayingSet(bool enable);
	void limitKeyLogRange();
	void onTimeout();
	void onCopyAction();
	void onSaveAction();
	void onTrayIconStateChange(bool isHideToTray);
	void addProgressBar(QString srcName, QString dstName, KoiFormat format);
	void onMoiveFrameChanged(int frameNumber);
	void onFrameConverted();
	void actionBarInit();
	void hotkeyInit();
	void close();
	QImage shotOnce();
	quint32 mPausetime = 0;
	float mRecordFPS = 0;
	QImage mShowFrame;
	QRect mGeoSave;
	const QImage *mShowImagePtr = nullptr; /* Show in main view */
    GoatScreenshot *mScreenShot = nullptr;
    KoiAnimation *mAnimation = nullptr;
	KoiConvert *mConvert = nullptr;
    int mLastUpdateInfoTime = 0;

	const QColor mRecordColor = QColor(230, 43, 26);
	const QColor mPauseColor = QColor(246, 138, 30);
	const QColor mPlayingColor = QColor(35, 209, 111);
	const QColor mProgressColor = QColor(230, 43, 26);
	QTimer mTimer;
	qint64 mRecodeTimeInv;
	qint64 mStartTime;
	qint64 mPlayedTime;

	ActionsBar* mActBar = nullptr;
	QLabel* mLblTime;
	QPushButton* mBtnRecord = nullptr;
	QPushButton* mBtnKeyLog = nullptr;
	QPushButton* mBtnPause = nullptr;
	QPushButton* mBtnSetting = nullptr;
	QPushButton* mBtnFormat = nullptr;
	QPushButton* mBtnClose = nullptr;
	QPushButton* mBtnInfo = nullptr;

	ActionsBar* mInforBar = nullptr;
	MoveControl* mMoveControl = nullptr;
	QLabel* mInforRectSize = nullptr;
	QLabel* mInfoFPS = nullptr;
	QLabel* mInfoFileSize = nullptr;

	QList<QWidget*> mEnbOnFinishList;

	OwlKeyLogLabel* mKeyLogLabel = nullptr;
	OwlMouseLogLabel* mMouseLogLabel = nullptr;
	OwlKeyHook* mKeyHook = nullptr;

	QThread* mHandleTread = nullptr;

	DeerShortcut *mShortcutRecord = nullptr;
	DeerShortcut *mShortcutPause = nullptr;
	DeerShortcut *mShortcutStop = nullptr;

	StatusCode mStatus = PrepareRecord;
	KoiMovie* mMovie = nullptr;
	PlayProgress *mPlayProgress = nullptr;
	bool mNeedPlay;
	QString mDelExclude;
	void onProgressMousePress(QMouseEvent *event);
	void onProgressMouseRelease(QMouseEvent *event);
	void onProgressValChangeRequest(int valRequest);

	quint64 mCapTimeStats = 0;
	struct {
		int capture;
		int encode;
	} mLostFrameStats;
	bool mIsAddedProgress = false;
};

