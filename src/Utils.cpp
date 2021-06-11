#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QDirIterator>
#include <QIcon>
#include <KoiAnimation.h>
#include <SalmonActions.h>
#include <Utils.h>
#include <QDebug>
#ifdef Q_OS_WIN
#include <shlobj_core.h>
#include <windows.h>
#include <Dwmapi.h>
#endif
bool OpenFolderAndSelectFile(QString pathIn)
{
#ifdef Q_OS_WIN
  HRESULT result = S_FALSE;
  const auto pidl = ::ILCreateFromPath(pathIn.replace("/","\\").toStdWString().c_str());
  if (pidl) {
    result = ::SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    ::ILFree(pidl);
  }
  return result == S_OK;
#else
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e") << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(pathIn.replace('\\', '/'));
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e") << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
    return true;
#endif 
}

QString addFilenameSuffix(QString fileName, QString Suffix)
{
    QFileInfo fInfo(fileName);
    return fInfo.path() + "/" + fInfo.baseName() + Suffix + "." + fInfo.suffix();
}

void clearFiles(const QString& path, quint32 overtime_days, QString Exclude)
{
  	QDir Dir(path);
    if(Dir.isEmpty() || !Dir.exists())
    {
        return;
    }
    QDirIterator DirsIterator(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while(DirsIterator.hasNext())
    {
        QString fileName = DirsIterator.next();
        if(Exclude == fileName)return;
        if(overtime_days){
            QFileInfo info(fileName);
            if(info.created().secsTo(QDateTime::currentDateTime()) < overtime_days * 24 * 60 *60)
            {
                continue;
            }
        }
        Dir.remove(fileName);
    }
}

QString FileSizeString(size_t size)
{
    const char* sizeUnit[] = { "b", "kb", "mb","gb" };
    size_t i;
    float sizef = size;
    for (i = 0; i < sizeof(sizeUnit); i++)
    {
        if (sizef < 1000)break;
        sizef /= 1024;
    }
    return QString("%1 %2").arg(QString::number(sizef, 'f', 2)).arg(sizeUnit[i]);
}

bool CheckIsEncodeLater(int format)
{
    if(format < 0){
        format = Opt_Int(_opt_RecordFormat);
    }
    switch (format)
    {
    case KoiFormat::GIF:
    case KoiFormat::APNG:
    case KoiFormat::WEBP:
        return Opt_Bool(_opt_EncoderLater);
    case KoiFormat::Dga:
        return false;
    default:
        return false;
    }
}

QIcon FormatIcon(int Format)
{
    if(Format < 0){
        Format = Opt_Int(_opt_RecordFormat);
    }
    switch (Format)
    {
    case KoiFormat::GIF:
        return SalActIcon::Gif();
    case KoiFormat::APNG:
        return SalActIcon::Png();
    case KoiFormat::WEBP:
        return SalActIcon::Webp();
    case KoiFormat::Dga:
        return SalActIcon::Dga();
    default:
        return SalActIcon::Gif();
    }
}

QRect GetWindowRect(QRect wRect)
{
#ifdef Q_OS_WIN
    POINT pos;
    QRect ret;
    RECT r1;
    pos.x = wRect.left() + wRect.width() / 2;
    pos.y = wRect.top() + wRect.height() / 2;
    HWND hwndPointNow = WindowFromPoint(pos); // 获取鼠标所在bai窗口的句柄
    /* 定位顶级窗口 */
    while(hwndPointNow){
	    DwmGetWindowAttribute(hwndPointNow, DWMWA_EXTENDED_FRAME_BOUNDS, &r1, sizeof(RECT));
        ret.setTop(r1.top+1);
        ret.setLeft(r1.left+1);
        ret.setRight(r1.right-2);
        ret.setBottom(r1.bottom-2);
        if(ret.contains(wRect)) return ret;
        LONG wStyle = GetWindowLong(hwndPointNow, GWL_STYLE);
        if((wStyle & WS_CHILDWINDOW) != WS_CHILDWINDOW){
            break;
        }
        hwndPointNow = GetParent(hwndPointNow);
    }
    return ret;
#else
    #todo: support other system
#endif
}