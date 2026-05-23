#include <QCoreApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QObject>

#include "qsnwebcontainer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec * codec  = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);

    QTranslator appTranslator;
    QCoreApplication::setOrganizationName("SignalNet");
    QCoreApplication::setApplicationName("snServer");

    QCoreApplication::setApplicationVersion(QLatin1String("0.10.19.2"));

    appTranslator.load(QLatin1String("ServerLng_")+QLocale::system().name(),"://");
    a.installTranslator(&appTranslator);

    QsnWebContainer WebContainer;

    return a.exec();
}
