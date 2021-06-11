#include "PlayProgress.h"
#include <QLinearGradient>
#include <QPainter>
#include <QMouseEvent>
PlayProgress::PlayProgress(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{        
    mLinear.setColorAt(0, QColor(204, 204, 204));
    mLinear.setColorAt(0.001, QColor(18, 194, 233));
    mLinear.setColorAt(0.5, QColor(196, 113, 237));
    mLinear.setColorAt(0.999, QColor(246, 79, 89));
    mLinear.setColorAt(1, QColor(204, 204, 204));
    // 设置显示模式
    mLinear.setSpread(QGradient::PadSpread);
    connect(&mTimer, &QTimer::timeout, this, &PlayProgress::onTimeout);
}

PlayProgress::~PlayProgress()
{
}

void PlayProgress::mousePressEvent(QMouseEvent *event)
{
    emit sigValMousePress(event);
    qint64 clkVal = mMinVal + (event->pos().x() / (qreal)this->width()) * (mMaxVal - mMinVal);
    emit sigValChangeRequest(clkVal);
}
void PlayProgress::mouseReleaseEvent(QMouseEvent *event)
{
    emit sigValMouseRelease(event);
}
void PlayProgress::mouseMoveEvent(QMouseEvent *event)
{
    qint64 clkVal = mMinVal + (event->pos().x() / (qreal)this->width()) * (mMaxVal - mMinVal);
    emit sigValChangeRequest(clkVal);
}
void PlayProgress::setMaxValue(int maxVal)
{
    mMaxVal = maxVal;
    mCutEnd = maxVal;
}

void PlayProgress::setValue(int Val)
{
    if(mIsDynGrowth){
        if(Val <= mValTemp){
            mValTemp = mVal;
        } else {
            mTimer.start(mDynGrowthParam / (Val - mValTemp));
        }
    }
    mVal = Val;
    this->update();
}

void PlayProgress::setStyle(ProgressStyle style)
{
    mProgressStyle = style;
    //mIsDynGrowth = mProgressStyle == Ring;
}

void PlayProgress::setMinValue(int Val)
{
    mMinVal = Val;
    mCutBegin = Val;
}

void PlayProgress::paintEvent(QPaintEvent* event)
{
    int val = mIsDynGrowth ? mValTemp : mVal;
    mLinear.setStart((mCutBegin - mMinVal) / (qreal)(mMaxVal - mMinVal) * this->width(), 1);
    mLinear.setFinalStop((val - mMinVal) / (qreal)(mMaxVal - mMinVal) * this->width(), 1);
    if(mProgressStyle == Strip){
        QPainter painter(this);
        painter.fillRect(QRectF(0,0,this->width(), this->height()), mLinear);
    } else {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int m_persent = (val - mMinVal) / (qreal)(mMaxVal - mMinVal) * 100;//绘制的环的比例
        int m_rotateAngle = 360*m_persent/100;
        int side = qMin(width(), height());
        QRectF outRect(((width() - side) / 2) + 0, ((height() - side) / 2) + 0, side, side);//矩形长宽为窗口的长宽
        QRectF inRect(((width() - side) / 2) + mRingWidth, ((height() - side) / 2) + mRingWidth, side-mRingWidth*2, side-mRingWidth*2);
        QString valueStr = QString("%1%").arg(QString::number(m_persent));
        //画外圆
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(97, 117, 118)));
        p.drawEllipse(outRect); 
        //画内圆
        p.setBrush(QBrush(QColor(0, 122, 204)));
        p.drawPie(outRect, (90-m_rotateAngle)*16, m_rotateAngle*16);
        //画遮罩，遮罩颜色为窗口颜色
        p.setBrush(palette().window().color());
        p.drawEllipse(inRect);
        //画文字
        QFont f = QFont("Microsoft YaHei", 15, QFont::Bold);
        p.setFont(f);
        p.setPen(QColor("#555555"));
        p.drawText(inRect, Qt::AlignCenter, valueStr);
    }
}

void PlayProgress::onTimeout()
{
    mValTemp++;
    update();
    if(mVal == mValTemp){
        mTimer.stop();
    }
}