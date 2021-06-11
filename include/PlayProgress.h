#include <QWidget>
#include <QTimer>
class PlayProgress : public QWidget
{
    Q_OBJECT
public:
    enum ProgressStyle{
        Strip,
        Ring
    };
    PlayProgress(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~PlayProgress();

    int maxValue(){return mMaxVal;};
    void setMaxValue(int maxVal);

    int value(){return mVal;};
    void setValue(int minVal);

    int minValue(){return mMinVal;};
    void setMinValue(int minVal);

    int cutBegin(){return mMaxVal;};
    void setCutBegin(int mCutBegin);

    int cutEnd(){return mCutEnd;};
    void setCutEnd(int CutEnd);

    void setStyle(ProgressStyle style);
signals:
    void sigValMousePress(QMouseEvent *event);
    void sigValMouseRelease(QMouseEvent *event);
    void sigValChangeRequest(int valRequest);
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int mMaxVal = 100;
    int mVal = 0;
    int mMinVal = 0;
    int mCutBegin = 0;
    int mCutEnd = 0;
    QLinearGradient mLinear;
    ProgressStyle mProgressStyle = Strip;
    QTimer mTimer;
    int mValTemp = 0;
    bool mIsDynGrowth = false;
    int mDynGrowthParam = 800;
    int mRingWidth = 15;
    void onTimeout();
};
