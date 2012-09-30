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
#include <QFileDialog>

#include "lib/TKN.h"
#include "lib/HEXParser.hpp"

TKN_NodeBox::TKN_NodeBox(QWidget *parent, int id) :
    QGroupBox(parent),
    ui(new Ui::TKN_NodeBox)
{
    ui->setupUi(this);

    /* Ui setup */
    this->NODE_ID = id;
    this->setTitle(QString("Node ").append(QString('0'+NODE_ID)));
    this->ui->labelAVR->setPixmap( QPixmap(":/AVR_Chip-W180px.png"));

    /* Signal connections */
    //connect(this, SIGNAL(dataReceived()), this, SLOT(on_dataReceived()));
    connect(this, SIGNAL(consoleOut(QString)),parentWidget(), SLOT(consoleOut(QString)), Qt::QueuedConnection);

    PAGESIZE = 128;
}

TKN_NodeBox::~TKN_NodeBox()
{
    delete ui;
}

void TKN_NodeBox::dataReceive(TKN_Data *data)
{
    receivedDataEcho(data);

    dataQueueMutex.lock();
    dataQueue.enqueue(*data);
    dataQueueMutex.unlock();
    dataQueueSem.release();

    emit dataReceived();
}

//void TKN_NodeBox::on_dataReceived()
//{
    // No action yet
//}

void TKN_NodeBox::receivedDataEcho(TKN_Data *data)
{
    ui->textEditConsole->append(QByteArray((char*)data, sizeof(TKN_Data)));
}

void TKN_NodeBox::on_buttonSend_clicked()
{
    TKN_Data data;
    qMemSet(&data, 0, sizeof(data));
    bool ok=false;
    ushort x = ui->lineEdit_DataToSend->text().toUShort(&ok, 16);
    if (ok){
        *((ushort*)(&data)) = x;
    }
    else {
        QByteArray bytes = ui->lineEdit_DataToSend->text().toAscii();
        memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    }

    TKN_PushData(&data, NODE_ID);
}

void TKN_NodeBox::on_buttonBrowseHex_clicked()
{
    QString newPath = QFileDialog::getOpenFileName(this);
    if (newPath!=NULL)
        ui->lineEditHexFilePath->setText(newPath);
}

void TKN_NodeBox::on_buttonHexUpload_clicked()
{
    QtConcurrent::run(this, &TKN_NodeBox::hexUpload);
}

void TKN_NodeBox::dataDeque(TKN_Data *data)
{
    dataQueueSem.acquire();
    dataQueueMutex.lock();
    *data = dataQueue.dequeue();
    dataQueueMutex.unlock();
}

void TKN_NodeBox::flushRecDataQueue()
{
    int av = dataQueueSem.available();

    if (av>0){
        dataQueueSem.acquire(av);
        dataQueueMutex.lock();
        dataQueue.clear();
        dataQueueMutex.unlock();
    }
}

void TKN_NodeBox::hexUpload()
{
    /* Flush any data */
    flushRecDataQueue();

    /* Do the job */
    int page=0;
    int word=0;
    int byteCount=0;
    long start, end, addr;
    TKN_Data recData;
    BYTE sendData[TKN_DATA_SIZE];
    BYTE byteForEmpty = 0xff;
    QString recString;

    if (ui->lineEditHexFilePath->text().size() < 5)
        return;

    HEXFile h = HEXFile(1024*1024, byteForEmpty);
    h.readFile(ui->lineEditHexFilePath->text().toStdString());

    start = h.getRangeStart();
    end   = h.getRangeEnd();

    for (addr=start; addr<= end; ) {
        /* Initiate a boot procedure */
        memset(sendData, 0, TKN_DATA_SIZE);
        strcpy ((char*)sendData, "B:");
        sendData[3] = addr/(PAGESIZE*2);

        emit consoleOut(QString("Page: ") + QString ('0'+sendData[3]));

        while (TKN_PushData ((TKN_Data *) sendData, NODE_ID));

        /* Wait MCU */
        do {
            dataDeque(&recData);
            recString = QString(QByteArray((char*)&recData, sizeof(TKN_Data)));
        } while ( QString::compare(QString("-MCU-READY------"), recString));

        /* Send a complete page */
        do {
            /* Fill a TKN Packet */
            for (int i=0; i<sizeof(TKN_Data); i++){
                sendData[i] = addr<=end? ((BYTE) h.getData(addr)): byteForEmpty;
                byteCount++;
                addr++;
            }

            /* Send it to tha MCU */
            while (TKN_PushData ((TKN_Data *) sendData, NODE_ID));

        } while ((addr % (PAGESIZE*2)!=0) );

    }

    QString msg = QString("Hex uploade complete. Sent ") + QString::number(byteCount) + QString(" bytes.");
    qDebug() << msg ;
    emit consoleOut(msg);
}
