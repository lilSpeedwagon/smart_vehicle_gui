#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    addressValidator = new AddressValidator();
    portValidator = new PortValidator();

    layout = new QVBoxLayout();
    {
        connectionBox = new QGroupBox("Connection");
        {
            connectionLayout = new QGridLayout();
            connectionLayout->setAlignment(Qt::AlignCenter);
            {
                addressLabel = new QLabel("Server adress");
                connectionLayout->addWidget(addressLabel, 0, 0);

                addressLine = new QLineEdit("127.0.0.1");
                addressLine->setPlaceholderText("Enter server adress here...");
                addressLine->setValidator(addressValidator);
                connectionLayout->addWidget(addressLine, 0, 1);

                portLabel = new QLabel("Port");
                connectionLayout->addWidget(portLabel, 1, 0);

                portLine = new QLineEdit("80");
                portLine->setPlaceholderText("Server port...");
                portLine->setValidator(portValidator);
                connectionLayout->addWidget(portLine, 1, 1);

                connectButton = new QPushButton("Start");
                connectionLayout->addWidget(connectButton, 2, 1, 1, 2);
            }
            connectionBox->setLayout(connectionLayout);
        }
    }
    layout->addWidget(connectionBox);
    {
        serverBox = new QGroupBox("Server");
        {
            serverLayout = new QGridLayout();
            serverLayout->setAlignment(Qt::AlignCenter);
            {

                messageLine = new QLineEdit();
                messageLine->setPlaceholderText("Enter message here...");
                serverLayout->addWidget(messageLine, 0, 0);

                sendButton = new QPushButton("Send");
                serverLayout->addWidget(sendButton, 0, 1);
            }
            serverBox->setLayout(serverLayout);
        }
    }
    layout->addWidget(serverBox);

    log = new QPlainTextEdit("Logs:");
    log->setReadOnly(true);
    layout->addWidget(log);

    setLayout(layout);

    connect(connectButton, SIGNAL(clicked()), this, SLOT(slotConnectButton()));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(slotSendButton()));

    slotServerChangeState(false);
}

MainWindow::~MainWindow()
{

}

void MainWindow::slotConnectButton()    {
    if (addressLine->isEnabled())    {
        QString address = addressLine->text();
        quint16 port = (quint16) portLine->text().toInt();
        if (!AddressValidator::isFull(address))
            QMessageBox::critical(nullptr, "Error", "Incorrect server address.");
        else
            emit signalServerStart(address, port);
    }   else {
        emit signalServerStop();
    }
}

void MainWindow::slotSendButton()   {
    QString data = messageLine->text();
    //validate
    emit signalServerSendAll(data);
    messageLine->clear();
}

void MainWindow::slotLog(QString message)   {
    log->appendPlainText(message);
}

void MainWindow::slotServerChangeState(bool state)    {
    if (state)  {
        connectButton->setText("Stop");
        addressLine->setEnabled(false);
        portLine->setEnabled(false);

        serverBox->setEnabled(true);
    }   else {
        connectButton->setText("Start");
        addressLine->setEnabled(true);
        portLine->setEnabled(true);

        serverBox->setEnabled(false);
    }
}

void MainWindow::slotRefreshData()  {

}