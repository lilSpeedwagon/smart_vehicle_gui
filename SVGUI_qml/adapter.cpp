#include "adapter.h"

Adapter::Adapter(QObject *parent) : QObject(parent) {
    speedSeriesFilter.setFilter(Filter::GAF);
    tempSeriesFilter.setFilter(Filter::GAF);
    tempSeriesFilter.setAutoScale(false);
    tempSeries.setAutoScale(false);
}

//converts state code into state string
QString Adapter::getStatusStr(const qint8 &state)  {
    QString stateString;
    switch (state)    {
        case 0: {
            stateString = "FAULT";
            break;
        }
        case 1: {
            stateString = "RUN";
            break;
        }
        case 2: {
            stateString = "STOP";
            break;
        }
        case 3: {
            stateString = "WAIT";
            break;
        }
        default:    {
            stateString = "UNKNOWN";
        }
    }
    return stateString;
}

void Adapter::clearCharts() {
    speedSeries.clear();
    speedSeriesFilter.clear();
    steeringSeries.clear();
    tempSeries.clear();
    tempSeriesFilter.clear();

    chartStartTime = 0;
    encoderLast = QPointF();
}


//show message in UI logger
void Adapter::log(const QString &message)   {
    emit signalUILog(message);
}

//gets a QLineSeries* from QML context
void Adapter::slotUISetSerieses(QObject *speedSeries, QObject* speedSeriesFilter, QObject *steeringSeries,
                                QObject *tempSeries, QObject* tempSeriesFilter)   {
    if (speedSeries)  {
        this->speedSeries.setSeriesObj(qobject_cast<QtCharts::QLineSeries*>(speedSeries));
        qDebug() << "Speed series has been initialized.";
    }   else
        qDebug() << "Speed series init error.";
    if (speedSeriesFilter)  {
        this->speedSeriesFilter.setSeriesObj(qobject_cast<QtCharts::QLineSeries*>(speedSeriesFilter));
        qDebug() << "Speed series (filter) has been initialized.";
    }   else
        qDebug() << "Speed series (filter) init error.";
    if (steeringSeries) {
        this->steeringSeries.setSeriesObj(qobject_cast<QtCharts::QLineSeries*>(steeringSeries));
        qDebug() << "Steering serieses has been initialized.";
    }   else
        qDebug() << "Steering series init error.";
    if (tempSeries) {
        this->tempSeries.setSeriesObj(qobject_cast<QtCharts::QLineSeries*>(tempSeries));
        qDebug() << "Temperature serieses has been initialized.";
    }   else
        qDebug() << "Temperature series init error.";
    if (tempSeriesFilter) {
        this->tempSeriesFilter.setSeriesObj(qobject_cast<QtCharts::QLineSeries*>(tempSeriesFilter));
        qDebug() << "Temperature filtered serieses has been initialized.";
    }   else
        qDebug() << "Temperature filtered series init error.";
}

//search for devices in current network
void Adapter::slotUISearch()    {
    emit signalSearch();
}

void Adapter::slotUIConnect(QString address, QString portStr)    {
    qDebug() << "Adapter: incoming connect signal";
    log("Connecting to " + address + "...");
    quint16 port = static_cast<quint16>(portStr.toInt());
    emit signalConnect(address, port);
}

void Adapter::slotUIDisconnect()    {
    emit signalDisconnect();
}

//getting settings from UI and converting them into SetPackage
void Adapter::slotUISettingsLoad(float steering_p, float steering_i, float steering_d, float steering_zero,
                                 float forward_p, float forward_i, float forward_d, float forward_int,
                                 float backward_p, float backward_i, float backward_d, float backward_int)  {
    log("Loading settings...");
    SetPackage set;

    set.steering_p = steering_p;
    set.steering_i = steering_i;
    set.steering_d = steering_d;
    set.steering_servoZero = steering_zero;

    set.forward_p = forward_p;
    set.forward_i = forward_i;
    set.forward_d = forward_d;
    set.forward_int = forward_int;

    set.backward_p = backward_p;
    set.backward_i = backward_i;
    set.backward_d = backward_d;
    set.backward_int = backward_int;

    emit signalSettingsLoad(set);
}

//requesting for Smart Vehicle settings
void Adapter::slotUISettingsUpload()    {
    log("Uploading settings...");
    emit signalSettingsUpload();
}

void Adapter::slotUIClearCharts()   {
    clearCharts();
}

//getting control data from UI and converting it into ControlPackage
void Adapter::slotUIControl(float const& xAxis, float const& yAxis) {
    ControlPackage data;
    data.xAxis = xAxis;
    data.yAxis = yAxis;
    emit signalControl(data);
}

//sets filter type and create new Filter instead of last choosen filter for every series
void Adapter::slotUISetFilter(int filterType)   {
    Filter::FilterType _filterType;
    switch (filterType) {
    case 0: {
        _filterType = Filter::NONE;
        break;
    }
    case 1: {
        _filterType = Filter::KALMAN;
        break;
    }
    case 2: {
        _filterType = Filter::GAF;
        break;
    }
    default:    {
        _filterType = Filter::NONE;
    }
    }
    speedSeriesFilter.setFilter(_filterType);
    tempSeriesFilter.setFilter(_filterType);
}

//sets K koef (only for Kalman)
void Adapter::slotUISetFilterK(float k) {
    speedSeriesFilter.setFilterK(k);
    tempSeriesFilter.setFilterK(k);
}

//gets a list of available network addresses
void Adapter::slotAddresses(QList<QString> const& addresses)    {
    emit signalUIAddresses(addresses);
}

void Adapter::slotConnected(qint8 const& state)   {
    qDebug() << "Adapter: Connected";
    emit signalUIConnected();
    emit signalUIStatus(getStatusStr(state));

    clearCharts();

    log("Connected.");
}

void Adapter::slotDisconnected()    {
    qDebug() << "Adapter: Disconnected";
    emit signalUIDisconnected();
    log("Disconnected.");
}

void Adapter::slotConnectionError(QString message) {
    qDebug() << "Adapter: Connection error";
    emit signalUIConnectionError();
    log("Connection error: " + message);
}

//calculate speed by delta time and delta value
float Adapter::getSpeed(float currentTime, float currentEncoder) {
    if (encoderLast.isNull())   {
        encoderLast = QPointF(currentTime, currentEncoder);
        return 0;
    }
    float speed = (currentEncoder - encoderLast.y()) / (currentTime - encoderLast.x());
    encoderLast = QPointF(currentTime, currentEncoder);
    return speed;
}

//gets new HighFreqDataPackage and extract all data from it to show in UI
void Adapter::slotData(HighFreqDataPackage const& data) {
    qDebug() << "Adapter: incoming high freq data package";

    if (!chartStartTime)
        chartStartTime = data.timeStamp;
    float deltaTime = (data.timeStamp - chartStartTime) / 1000.0f;

    float speed = getSpeed(deltaTime, data.m_encoderValue);

    emit signalUIUpdateHighFreqData(data.m_encoderValue, data.m_steeringAngle, speed);
    emit signalUIUpdatePosition(data.x, data.y, data.angle);

    speedSeries.addPoint(QPointF(deltaTime, speed));
    speedSeriesFilter.addPoint(QPointF(deltaTime, speed));
    steeringSeries.addPoint(QPointF(deltaTime, data.m_steeringAngle));
}

//gets new LowFreqDataPackage and extract all data from it to show in UI
void Adapter::slotData(LowFreqDataPackage const& data) {
    qDebug() << "Adapter: incoming low freq data package";

    qint8 state = data.stateType;
    QString stateString = getStatusStr(state);

    emit signalUIStatus(stateString);
    emit signalUIUpdateLowFreqData(data.m_motorBatteryPerc, data.m_compBatteryPerc, data.m_temp);

    if (!chartStartTime)
        chartStartTime = data.timeStamp;
    float deltaTime = (data.timeStamp - chartStartTime) / 1000.0f;

    QPointF point(deltaTime, data.m_temp);
    tempSeries.addPoint(point);
    tempSeriesFilter.addPoint(point);
}

//gets result of settings applying
void Adapter::slotDone(qint8 const& answerCode) {
    switch (answerCode) {
    case 0: {
        log("Error. Can't apply current settings.");
        break;
    }
    case 1: {
        log("Setting complete.");
        break;
    }
    }
}

//gets settings from Smart Vehicle to transfer them into UI
void Adapter::slotSettings(SetPackage const& set)   {
    emit signalUISettings(set.steering_p, set.steering_i, set.steering_d, set.steering_servoZero,
                          set.forward_p, set.forward_i, set.forward_d, set.forward_int,
                          set.backward_p, set.backward_i, set.backward_d, set.backward_int);
    log("Settings uploaded.");
}

//gets MapPackage to create a cell map in UI
void Adapter::slotMap(MapPackage const& map)    {
    QList<int> cellsList;
    for (auto const& line : map.cells())
        for (auto const& cell : line)
            cellsList.push_back(cell);

    emit signalUIMap(map.getWidth(), map.getHeight(), cellsList);
}

void Adapter::slotBrokenPackage()   {
    log("Incoming broken package.");
}
