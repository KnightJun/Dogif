#include <QDialog>
#include <QTranslator>
class QSettings;
class Ui_OptionWindow;
class QGraphicsOpacityEffect;
class QPropertyAnimation;
class OptionWindow : public QDialog
{
    Q_OBJECT
private:
    /* data */
public:
    OptionWindow(QWidget *parent = nullptr);
    ~OptionWindow();
	static void setOptionSetting(QSettings* setting);
    static void InitUiByLanguage();
public slots:
    void onOptionChanged(int newVal);
    void onOptionChanged(QString newVal);
    void onOptionChanged(bool newVal);
    void onSaved();
    void onTimeFormatChanged(QString text);
    void on_btnCheckUpdate_clicked();
    void onLanguageChanged(QString);
    void onLosslessChanged(bool);
    void aboutQt();

    void on_btnOpenFloder_clicked();
    void on_OptHistoryFilePath_clicked();
signals:
    void sigFormatChanged();
    
private:
    Ui_OptionWindow *ui;
    QGraphicsOpacityEffect *mSaveOpacityEffect;
    QPropertyAnimation *mSaveOpacityAnimation1;

    void InitLanguageCombox();
};

