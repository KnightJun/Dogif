#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <qdebug.h>
#include <DynamicShotWidget.h>
#include <OptionWindow.h>
#include <QMainWindow>
#include <Utils.h>
#include <RobinLog.h>
DynamicShotWidget::DynamicShotWidget(const QRect geo, QWidget* parent)
	:QMainWindow(parent)
{
    if(geo.isEmpty()){
        QScreen* screen = QGuiApplication::primaryScreen();
        mShotRect = screen->geometry();
        mShotRect -= QMargins(mShotRect.width() / 3 , mShotRect.height() / 3, 
            mShotRect.width() / 3, mShotRect.height() / 3);
        adjustShotRect(mShotRect);
    }else{
	    adjustShotRect(geo);
    }
	//this->setStyleSheet("background-color: rgb(0, 0, 0);");
	this->setMouseTracking(true);
	setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	mPen = QPen(mNormalColor, mPenWidth);
	mPen.setJoinStyle(Qt::MiterJoin);
    LOG_INFO("DynamicShotWidget Init");
} 

DynamicShotWidget::~DynamicShotWidget()
{
}

void DynamicShotWidget::staysOnTop(bool enable)
{
    if(enable == mStaysOnTop)return;
    mStaysOnTop = enable;
    this->setWindowFlag(Qt::WindowStaysOnTopHint, enable);
    this->show();
    emit SigStaysOnTopChanged(enable);
}

bool DynamicShotWidget::event(QEvent *event)
{
    if(QEvent::WindowDeactivate == event->type())
    {
        emit SigWindowDeactivate();
    }else if (QEvent::WindowActivate == event->type()){
        emit SigWindowActivate();
    }
    return QWidget::event(event);
}

void DynamicShotWidget::showTaskIcon(bool enable)
{
    if(enable == mShowTaskIcon) return;
    this->setWindowFlag(Qt::Tool, !enable);
    this->show();
    mShowTaskIcon = enable;
}

void DynamicShotWidget::fitsWindow()
{
    if(mFitsWinFlag){
        LOG_INFO("restore shot rect to {}", mAdjustRecord);
        adjustShotRect(mAdjustRecord);
        return;
    }
    QRect geo = this->geometry();
    QRect wRect = GetWindowRect(geo);
    LOG_INFO("fit shot rect to {}", wRect);
    mAdjustRecord = mShotRect;
    if(!wRect.isEmpty()){
        adjustShotRect(wRect);
    }
    mFitsWinFlag = true;
}
void DynamicShotWidget::adjustShotRect(const QRect &rect)
{
	mShotRect = rect.intersected(
        QGuiApplication::primaryScreen()->geometry().marginsRemoved(
            QMargins(1,1,1,1)
        ));

    mMargin = QMargins(mPenWidth, mPenWidth, mPenWidth, mPenWidth);
	mShotRectOnDialog.setRect(mPenWidth, mPenWidth, mShotRect.width(), mShotRect.height());
	mDrawRect = mShotRectOnDialog.marginsAdded(mMargin / 2);
	mBoundRect = mDrawRect.marginsAdded(mMargin / 2);
	this->resize(mBoundRect.size());
	this->move(QPoint(mShotRect.x() - mPenWidth, mShotRect.y() - mPenWidth));
}

void DynamicShotWidget::adjustNegRect(QRect& tempRect)
{

    if (tempRect.width() < 0) {
        int tempWidth = -tempRect.width();
        if ((mZoomFlag & 0xf0) == 0x10)
        {
            mZoomFlag &= 0x0f;
            mZoomFlag |= 0x30;
            tempRect.setLeft(tempRect.right());
            tempRect.setWidth(tempWidth);
        }
        else if ((mZoomFlag & 0xf0) == 0x30)
        {
            mZoomFlag &= 0x0f;
            mZoomFlag |= 0x10;
            tempRect.setLeft(tempRect.left() - tempWidth);
            tempRect.setWidth(tempWidth);
        }
    }
    if (tempRect.height() < 0) {
        int tempHeight = -tempRect.height();
        if ((mZoomFlag & 0x0f) == 0x01)
        {
            mZoomFlag &= 0xf0;
            mZoomFlag |= 0x03;
            tempRect.setTop(tempRect.bottom());
            tempRect.setHeight(tempHeight);
        }
        else if ((mZoomFlag & 0x0f) == 0x03)
        {
            mZoomFlag &= 0xf0;
            mZoomFlag |= 0x01;
            tempRect.setTop(tempRect.top() - tempHeight);
            tempRect.setHeight(tempHeight);
        }
    }
}

void DynamicShotWidget::changeBoardColor(QColor color)
{
    mPen.setColor(color.isValid()? color: mNormalColor);
    update();
}

void DynamicShotWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (mLockGeo) {
        this->setCursor(Qt::ArrowCursor);
        return;
    }
	if (mIsPress)
	{
        QPoint ptemp = event->globalPos();
        ptemp = ptemp - mPressPoint;
        if (mZoomFlag == 22)//移动窗口
        {
            ptemp = ptemp + pos();
            move(ptemp);
        }
        else
        {
            QRect tempRect = geometry();
            switch (mZoomFlag)//改变窗口的大小
            {

            case 0x11:tempRect.setTopLeft(tempRect.topLeft() + ptemp); break;//左上角
            case 0x31:tempRect.setTopRight(tempRect.topRight() + ptemp); break;//右上角
            case 0x13:tempRect.setBottomLeft(tempRect.bottomLeft() + ptemp); break;//左下角
            case 0x33:tempRect.setBottomRight(tempRect.bottomRight() + ptemp); break;//右下角
            case 0x21:tempRect.setTop(tempRect.top() + ptemp.y()); break;//中上角
            case 0x12:tempRect.setLeft(tempRect.left() + ptemp.x()); break;//中左角
            case 0x32:tempRect.setRight(tempRect.right() + ptemp.x()); break;//中右角
            case 0x23:tempRect.setBottom(tempRect.bottom() + ptemp.y()); break;//中下角
            }
            if (abs(tempRect.width()) < 16 + mPenWidth * 2)return;
            if (abs(tempRect.height()) < 16 + mPenWidth * 2)return;
            adjustNegRect(tempRect);
            mShotRectOnDialog.setSize(mShotRect.size());
            mDrawRect = mShotRectOnDialog.marginsAdded(mMargin / 2);
            setGeometry(tempRect);
            this->onGeometryChanged();
        }

        mPressPoint = event->globalPos();//更新位置
	}
	else {
        if (event->modifiers() & Qt::ControlModifier) {
            mZoomFlag = 22;
        }else{
            mZoomFlag = calcBroadFlag(event->pos());
        }
		setCursorType(mZoomFlag);
	}
}
void DynamicShotWidget::mousePressEvent(QMouseEvent* event)
{
	mIsPress = true;
    mPressPoint = event->globalPos();
    mZoomFlag = calcBroadFlag(event->pos());
    onMousePress(mPressPoint, mZoomFlag);
}
void DynamicShotWidget::mouseReleaseEvent(QMouseEvent* event)
{
	mIsPress = false;
}
void DynamicShotWidget::setCursorType(int flag)//根据鼠标所在位置改变鼠标指针形状
{
    Qt::CursorShape cursor;
    switch (flag)
    {
    case 0x11:
    case 0x33:
        cursor = Qt::SizeFDiagCursor; break;
    case 0x13:
    case 0x31:
        cursor = Qt::SizeBDiagCursor; break;
    case 0x12:
	case 0x32:
        cursor = Qt::SizeHorCursor; break;
    case 0x21:
	case 0x23:
        cursor = Qt::SizeVerCursor; break;
    case 0x22:
        //cursor = Qt::SizeAllCursor; break;
    default:
        cursor = Qt::ArrowCursor; break;
        break;

    }
    setCursor(cursor);
}

void DynamicShotWidget::move(const QPoint& pos)
{
    QPoint gpos = QWidget::mapToGlobal(this->pos());
    QRect deskRt = QApplication::desktop()->screenGeometry(gpos);
    QPoint tpos = pos;
    if (pos.x() < -mPenWidth)tpos.setX(-mPenWidth + 1);
    if (pos.y() < -mPenWidth)tpos.setY(-mPenWidth + 1);
    if (pos.x() + this->width() > deskRt.width() + mPenWidth)
        tpos.setX(deskRt.width() + mPenWidth - this->width() - 1);
    if (pos.y() + this->height() > deskRt.height() + mPenWidth)
        tpos.setY(deskRt.height() + mPenWidth - this->height() - 1);
    QMainWindow::move(tpos);
    this->onGeometryChanged();
}

void DynamicShotWidget::onGeometryChanged()
{
    mFitsWinFlag = false;
    mShotRect = this->geometry().marginsRemoved(mMargin);
    emit SigGeometryChanged(this->geometry());
}

void DynamicShotWidget::paintEvent(QPaintEvent* event)
{
    if(mShowBorder){
        QPainter painter(this);
        //painter.fillRect(this->geometry(), QBrush(Qt::transparent));
        painter.setPen(mPen);
        painter.drawRect(mDrawRect);
    }
}

void DynamicShotWidget::showBorder(bool enable)
{
    mShowBorder = enable;
    update();
}

uint DynamicShotWidget::calcBroadFlag(const QPoint& pot)
{
	uint flat = 0;
	if (pot.x() < mShotRectOnDialog.x())flat += 0x10;
	else if(pot.x() < mShotRectOnDialog.right()) flat += 0x20;
	else  flat += 0x30;
	if (pot.y() < mShotRectOnDialog.y())flat += 1;
	else if (pot.y() < mShotRectOnDialog.bottom()) flat += 2;
	else  flat += 3;
	return flat;
}
