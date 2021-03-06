#include <QApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QQmlContext>
#include "svclient.h"
#include "adapter.h"

//Init signals/slots connecions between network client and adapter objects
void initConnections(SVClient *client, Adapter *adapter)  {
    QObject::connect(adapter, SIGNAL(signalConnect(QString const&, quint16 const&)), client, SLOT(slotUIConnect(QString const&, quint16 const&)));
    QObject::connect(adapter, SIGNAL(signalDisconnect()), client, SLOT(slotUIDisconnect()));
    QObject::connect(adapter, SIGNAL(signalSearch()), client, SLOT(slotUISearch()));
    QObject::connect(adapter, SIGNAL(signalSettingsLoad(SetPackage const&)), client, SLOT(slotUISettingsLoad(SetPackage const&)));
    QObject::connect(adapter, SIGNAL(signalSettingsUpload()), client, SLOT(slotUISettingsUpload()));
    QObject::connect(adapter, SIGNAL(signalControl(ControlPackage const&)), client, SLOT(slotUIControl(ControlPackage const&)));

    QObject::connect(client, SIGNAL(signalUIAddresses(QList<QString> const&)), adapter, SLOT(slotAddresses(QList<QString> const&)));
    QObject::connect(client, SIGNAL(signalUIConnected(qint8 const&)), adapter, SLOT(slotConnected(qint8 const&)));
    QObject::connect(client, SIGNAL(signalUIDisconnected()), adapter, SLOT(slotDisconnected()));
    QObject::connect(client, SIGNAL(signalUIError(QString)), adapter, SLOT(slotConnectionError(QString)));
    QObject::connect(client, SIGNAL(signalUIData(LowFreqDataPackage const&)), adapter, SLOT(slotData(LowFreqDataPackage const&)));
    QObject::connect(client, SIGNAL(signalUIData(HighFreqDataPackage const&)), adapter, SLOT(slotData(HighFreqDataPackage const&)));
    QObject::connect(client, SIGNAL(signalUIDone(qint8 const&)), adapter, SLOT(slotDone(qint8 const&)));
    QObject::connect(client, SIGNAL(signalUISettings(SetPackage const&)), adapter, SLOT(slotSettings(SetPackage const&)));
    QObject::connect(client, SIGNAL(signalUIMap(MapPackage const&)), adapter, SLOT(slotMap(MapPackage const&)));
    QObject::connect(client, SIGNAL(signalUIBrokenPackage()), adapter, SLOT(slotBrokenPackage()));
}

int main(int argc, char *argv[])
{
    qDebug() << "Application initializing...";
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication guiApp(argc, argv);

    SVClient *client = new SVClient();

    Adapter *adapter = new Adapter();

    initConnections(client, adapter);

    qDebug() << "User interface initializing...";
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("adapter", adapter);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    qDebug() << "Done. User interface is ready.";

    qDebug() << "Done. Aplication has been initialized and ready to work.";
    qDebug() << "----------------------------------------------";

    return guiApp.exec();
}
