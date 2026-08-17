#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include "MainWindow.h"

static void WriteAppLog(const QString& msg)
{
    QFile log(QCoreApplication::applicationDirPath() + "/diagnostic.log");
    if (log.open(QIODevice::Append | QIODevice::Text))
    {
        log.write(QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toUtf8());
        log.write(" ");
        log.write(msg.toUtf8());
        log.write("\n");
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    WriteAppLog("[APP] main enter");

    app.setApplicationName("PscanTestClient");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("TransCom");

    WriteAppLog("[APP] before MainWindow ctor");
    MainWindow mainWindow;
    WriteAppLog("[APP] after MainWindow ctor");

    mainWindow.show();
    WriteAppLog(QString("[APP] after show visible=%1 title=%2")
        .arg(mainWindow.isVisible())
        .arg(mainWindow.windowTitle()));

    const int rc = app.exec();
    WriteAppLog(QString("[APP] app exit rc=%1").arg(rc));
    return rc;
}
