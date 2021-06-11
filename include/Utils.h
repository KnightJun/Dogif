#pragma once
#include <QKeySequence>
#include <QSettings>

extern QSettings *GlobalOption;
static const char* _opt_RecordFormat = "OptRecordFormat";
static const char* _opt_HistoryFilePath = "OptHistoryFilePath";
static const char* _opt_FileTimeFormat = "OptFileTimeFormat";
static const char* _opt_AutoCleanFile = "OptAutoCleanFile";
static const char* _opt_OnlyCleanOvertimeFile = "OptOnlyCleanOvertimeFile";
static const char* _opt_CleanFileOverdays = "OptCleanFileOverdays";
static const char* _opt_HotkeyRecord = "OptHotkeyRecord";
static const char* _opt_HotkeyPause = "OptHotkeyPause";
static const char* _opt_CloseAfterCopy = "OptCloseAfterCopy";
static const char* _opt_CloseAfterSave = "OptCloseAfterSave";
static const char *_opt_keeplastpos = "OptKeepLastPos";
static const char *_opt_SecondaryMode = "OptSecondaryMode";
static const char *_opt_MaxFPS = "OptMaxFPS";
static const char *_opt_AddProgressBar = "OptAddProgressBar";
static const char *_opt_EncoderLater = "OptEncoderLater";
static const char* _opt_Language = "OptLanguage";
#define Opt_SetDef(key, val) \
    if(! GlobalOption->contains(key)){ \
         GlobalOption->setValue(key, val); \
    }
    
#define Opt_SetDefKeySeq(key, KeySeq) \
    if(! GlobalOption->contains(key)){ \
         GlobalOption->setValue(key, KeySeq.toString()); \
    }
#define Opt_Bool(key)   (bool) GlobalOption->value(key).toInt()
#define Opt_Int(key)     GlobalOption->value(key).toInt()
#define Opt_Str(key)     GlobalOption->value(key).toString()
#define Opt_KeySeq(key) QKeySequence::fromString(Opt_Str(key))

#define Opt_SetVal(key, val) GlobalOption->setValue(key, val); 

QString addFilenameSuffix(QString fileName, QString Suffix);
QString FileSizeString(size_t size);
QIcon FormatIcon(int Format = -1);
bool OpenFolderAndSelectFile(QString pathIn);
void clearFiles(const QString& path, quint32 overtime_days, QString Exclude);
QRect GetWindowRect(QRect);
bool CheckIsEncodeLater(int format = -1);