#pragma once
#include <qwidget.h>
#include <QDialog>
#include <QPainter>
#include <QTimer>
#include <QTime>
#include <qpushbutton.h>
#include <QMainWindow>
#define SizeString(size) QString("%1 x %2").arg(size.width()).arg(size.height())

#ifndef BUILD_STATIC
# if defined(DynamicShotWidget_LIB)
#  define DynamicShotWidget_EXPORT Q_DECL_EXPORT
# else
#  define DynamicShotWidget_EXPORT Q_DECL_IMPORT
# endif
#else
# define DynamicShotWidget_EXPORT
#endif

class DynamicShotWidget_EXPORT DynamicShotWidget :
    public QMainWindow
{
    Q_OBJECT
public:
    DynamicShotWidget(const QRect geo = QRect(), QWidget* parent = nullptr);
    virtual ~DynamicShotWidget();
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event)override;
    void setCursorType(int flag);
    void move(const QPoint &pos);
    void adjustNegRect(QRect &rect);
    void adjustShotRect(const QRect &rect);
    void changeBoardColor(QColor color = QColor());
	void fitsWindow();
    void staysOnTop(bool enable);
    void showBorder(bool enable);
    void showTaskIcon(bool enable);
    QRect mShotRect;
    bool mLockGeo = false;
Q_SIGNALS:
    void SigGeometryChanged(const QRect geoRect);
    void SigStaysOnTopChanged(bool enable);
    void SigWindowDeactivate();
    void SigWindowActivate();

protected:
    void paintEvent(QPaintEvent* event);
    bool event(QEvent *event) override;
    virtual void onMousePress(const QPoint& pot, int zoomflag){};
    QRect mShotRectOnDialog;
    const int mPenWidth = 6;
    QColor mNormalColor = QColor(0, 122, 204);
    QPen mPen;
    QRect mDrawRect;
    QRect mBoundRect;
    
public:
    void onGeometryChanged();

private:
    uint calcBroadFlag(const QPoint& pot);

    QMargins mMargin;
    int mZoomFlag = 0;
    bool mIsPress = false;
    QPoint mPressPoint;
    bool mFitsWinFlag = false;
    QRect mAdjustRecord;
    bool mShowBorder = true;
    bool mStaysOnTop = true;
    bool mShowTaskIcon = false;
};

