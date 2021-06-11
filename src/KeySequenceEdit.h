#pragma once
#include <qlineedit.h>
#include <QKeySequence>
class QSettings;
class DeerShortcut;
typedef bool (*HotkeySetCallback)(const QKeySequence keyseq);
class KeySequenceEdit :
    public QLineEdit
{
    Q_OBJECT
public:
    KeySequenceEdit(QWidget *parent = nullptr);
    ~KeySequenceEdit();
	static void setOptionSetting(QSettings* setting);
    void settingInit(const QString &objectName);
public slots:
    void setText(const QString &){};
    void setStyleSheet(const QString& styleSheet){};
    void cleanSetting();
protected:
    void keyPressEvent(QKeyEvent* event) override;
signals:
    void SettingFinish(const QKeySequence keyseq);
private:
    HotkeySetCallback mSetCallback = nullptr;
    static QSettings* mSettings;
    DeerShortcut*   mDeerShortcut;
    QKeySequence mkeySeq; 
    bool mIsOnly;
};

