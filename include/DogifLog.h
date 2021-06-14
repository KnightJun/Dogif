#pragma once
#include "spdlog/spdlog.h"

#include <spdlog/fmt/ostr.h> // must be included
#include "spdlog/sinks/rotating_file_sink.h"
#include <QDebug>
#include <iostream>

static std::shared_ptr <spdlog::logger> g_logger;
static void QtLogOutputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString text;
    switch(type)
    {
    case QtDebugMsg:
        g_logger->debug("{}", msg.toStdString());
        break;
    case QtWarningMsg:
        g_logger->warn("{}", msg.toStdString());
        break;
    case QtInfoMsg:
        g_logger->info("{}", msg.toStdString());
        break;
    case QtFatalMsg:
    case QtCriticalMsg:
        g_logger->error("{}", msg.toStdString());
        break;
    }
}

static void InstallQtLogToRobin(const char * logFile,
int maxSize = 1048576 * 1, int maxFiles = 2){
    g_logger = spdlog::rotating_logger_mt("DogifLog", logFile, maxSize, maxFiles);
    qInstallMessageHandler(QtLogOutputMessage);
}
