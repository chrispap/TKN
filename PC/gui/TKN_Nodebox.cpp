#include "TKN_Nodebox.h"
#include "ui_TKN_Nodebox.h"
#include "TKN_Window.h"

#include <QPixmap>
#include <QDebug>
#include <QThread>
#include <QtConcurrentRun>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "lib/TKN.h"

TKN_NodeBox::TKN_NodeBox(QWidget *parent, int id) :
    QGroupBox(parent),
    ui(new Ui::TKN_NodeBox)
{
    ui->setupUi(this);

    /* Ui setup */
    this->node_id = id;
    this->setTitle(QString("Node ").append(QString('0'+node_id)));
    this->ui->labelAVR->setPixmap( QPixmap(":/AVR_Chip-W180px.png"));

    /* Signal connections */
    connect(this, SIGNAL(dataReceived()), this, SLOT(on_dataReceived()));

}

TKN_NodeBox::~TKN_NodeBox()
{
    delete ui;
}

void TKN_NodeBox::dataReceive(TKN_Data *data)
{
    int x = *((int *) data);
    ui->textEditConsole->append(QString::number(x));

//    consoleOut(data);
    dataQueueMutex.lock();
    dataQueue.enqueue(*data);
    dataQueueMutex.unlock();
    dataQueueSem.release();
    emit dataReceived();
}

void TKN_NodeBox::on_dataReceived()
{
    //...
}

void TKN_NodeBox::consoleOut(TKN_Data *data)
{
    ui->textEditConsole->append(QByteArray((char*)data, sizeof(TKN_Data)));
}

void TKN_NodeBox::on_buttonSend_clicked()
{
    TKN_Data data;
    qMemSet(&data, 0, sizeof(data));
    QByteArray bytes = ui->lineEdit_DataToSend->text().toAscii();
    memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    TKN_PushData(&data, node_id);
}

void TKN_NodeBox::on_buttonSendFile_clicked()
{
    QtConcurrent::run(this, &TKN_NodeBox::sendFile);
}

void TKN_NodeBox::sendFile()
{
    /* Ensure that the data Qeueue has no elements*/
    int av = dataQueueSem.available();

    if (av>0){
        dataQueueSem.acquire(av);
        dataQueueMutex.lock();
        dataQueue.clear();
        dataQueueMutex.unlock();
    }

    /* Do the job */
    int packC=0;
    int r;
    TKN_Data recData;
    QString recString;

    do {
        while (TKN_PushData ((TKN_Data *) "__From Laptop__", node_id));

        do {
            dataQueueSem.acquire();
            dataQueueMutex.lock();
            recData = dataQueue.dequeue();
            dataQueueMutex.unlock();
            recString = QString(QByteArray((char*)&recData, sizeof(TKN_Data)));

        } while ( !QString::compare(QString("-MCU-READY------"), recString));

        packC++;
    } while (packC<10);

    qDebug() << "File Sent.";
}
