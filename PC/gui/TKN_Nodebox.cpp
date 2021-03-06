#include "TKN_Nodebox.h"
#include "ui_TKN_Nodebox.h"
#include "TKN_Window.h"

#include <QPixmap>
#include <QDebug>
#include <QThread>
#include <QtConcurrent/QtConcurrentRun>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>
#include <QFileDialog>
#include <qwt_dial.h>
#include <qwt_dial_needle.h>

#include "lib/TKN.h"
#include "lib/HEXParser.hpp"
#include "lib/ErrorMsg.hpp"

TKN_NodeBox::TKN_NodeBox(QWidget *parent, int id) :
    NODE_ID(id),
    voltometer(this),
    QGroupBox(parent),
    ui(new Ui::TKN_NodeBox)
{
    ui->setupUi(this);
    setTitle(QString("Node ").append(QString('0'+NODE_ID)));

    /* Configure the voltometer */
    ui->groupBox_adc->layout()->addWidget(&voltometer);
    voltometer.setReadOnly(true);
    voltometer.setMode(QwtDial::RotateNeedle);
    voltometer.setScaleArc(30, 330);
    voltometer.setScale(0.0, 5.0);
    voltometer.setNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow));

    /* Signal connections */
    connect(this, SIGNAL(dataReceived(TKN_Data)), this, SLOT(dataEcho(TKN_Data)));
    connect(this, SIGNAL(dataReceived(TKN_Data)), this, SLOT(voltometerSet(TKN_Data)));
    connect(this, SIGNAL(consoleOut(QString)), parentWidget(), SLOT(consoleOut(QString)), Qt::QueuedConnection);

    PAGESIZE = 128;
}

TKN_NodeBox::~TKN_NodeBox()
{
    delete ui;
}

void TKN_NodeBox::receiveData(TKN_Data *data)
{
    dataQueueMutex.lock();
    dataQueue.enqueue(*data);
    dataQueueMutex.unlock();
    dataQueueSem.release();
    emit dataReceived(*data);
}

void TKN_NodeBox::voltometerSet(TKN_Data data)
{
    if (data.data[0]=='A' && data.data[1]==':') {
        int AD = (int) strtol((const char*)&data.data[2], NULL, 16);
        voltometer.setValue((float)AD/255.*5.);
    }
    return;
}

void TKN_NodeBox::dataEcho(TKN_Data data)
{
    if (!ui->checkBox_log->isChecked()) return;

    if (ui->radioButton_ascii->isChecked())
        ui->textEditConsole->append(QByteArray((char*)&data, sizeof(TKN_Data)));
    else {
        QString line = "HEX: ";
        QString b;
        int i;
        for (i=0; i<TKN_DATA_SIZE/2-1; i++){
            b = QString::number(data.data[i],16).toUpper();
            if (b.length()==1) line += "0" + b; else line += b;
            line += "|";
        }
        b = QString::number(data.data[i++],16).toUpper();
        if (b.length()==1) line += "0" + b; else line += b;

        ui->textEditConsole->append(line);
        line = "     ";

        for ( ; i<TKN_DATA_SIZE-1; i++){
            QString b = QString::number(data.data[i],16).toUpper();
            if (b.length()==1) line += "0" + b; else line += b;
            line += "|";
        }
        b = QString::number(data.data[i],16).toUpper();
        if (b.length()==1) line += "0" + b; else line += b;

        ui->textEditConsole->append(line);
    }
}

void TKN_NodeBox::on_buttonSend_clicked()
{
    TKN_Data data;
    memset(&data, 0, sizeof(data));
    char *inv_ptr=0;
    unsigned long x = strtoul (ui->lineEdit_DataToSend->text().toLatin1().data() , &inv_ptr, 0);
    if (!*inv_ptr){
        *((unsigned long*)(&data)) = x;
    }
    else {
        QByteArray bytes = ui->lineEdit_DataToSend->text().toLatin1();
        memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    }

    TKN_PushData(&data, NODE_ID);
}

void TKN_NodeBox::on_buttonBrowseHex_clicked()
{
    QString dir = ui->lineEditHexFilePath->text().length()>5?ui->lineEditHexFilePath->text(): "C:/Users/Chris/Documents/GitHub/TKN/AVR/asm_sample/Debug";
    QString newPath = QFileDialog::getOpenFileName(this, "Open hex file", dir, "*.hex");
    if (newPath!=NULL)
        ui->lineEditHexFilePath->setText(newPath);
}

void TKN_NodeBox::on_buttonUpload_clicked()
{
    QtConcurrent::run(this, &TKN_NodeBox::hexUpload);
}

void TKN_NodeBox::on_buttonRun_clicked()
{
    TKN_Data data;
    memcpy((void*)data.data, (void*)"R:", 3);
    TKN_PushData(&data, NODE_ID);
}

void TKN_NodeBox::on_horizontalSlider_valueChanged(int value)
{
    if (value==0) return;

    TKN_Data data;
    memset(&data, 0, sizeof(data));
    *((ushort*)(&data)) = 255 * value/ui->horizontalSlider->maximum();
    TKN_PushData(&data, NODE_ID);
}

void TKN_NodeBox::on_checkBox_log_toggled(bool checked)
{
    ui->radioButton_ascii->setEnabled(checked);
    ui->radioButton_hex->setEnabled(checked);
}

bool TKN_NodeBox::dataDeque(TKN_Data *data, int timeout)
{
    if (dataQueueSem.tryAcquire(1, timeout)) {
        dataQueueMutex.lock();
        *data = dataQueue.dequeue();
        dataQueueMutex.unlock();
        return true;
    }
    else
        return false;
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
    int page=0;
    int word=0;
    int byteCount=0;
    long start, end, addr;
    TKN_Data recData;
    BYTE sendData[TKN_DATA_SIZE];
    BYTE byteForEmpty = 0xff;
    QString recString;

    HEXFile *h;

    try {
        h = new HEXFile(1024*1024, byteForEmpty);
        h->readFile(ui->lineEditHexFilePath->text().toStdString());
    } catch(ErrorMsg *err) {
        emit consoleOut(QString::fromStdString(err->What()));
        return;
    }

    start = h->getRangeStart();
    end   = h->getRangeEnd();

    for (addr=start; addr<= end; ) {
        /* Initiate a boot procedure */
        memset(sendData, 0, TKN_DATA_SIZE);
        strcpy ((char*)sendData, "B:");
        sendData[3] = addr/(PAGESIZE*2);

        /* Flush any data from receive queue */
        flushRecDataQueue();

        while (TKN_PushData ((TKN_Data *) sendData, NODE_ID));

        /* Wait MCU */
        do {
            if ( !dataDeque(&recData, 1500)) {
                emit consoleOut("MCU Busy, aborting.");
                return;
            }
            recString = QString(QByteArray((char*)&recData, sizeof(TKN_Data)));
        } while ( QString::compare(QString("-MCU-READY------"), recString));

        emit consoleOut(QString("Sending Page: ") + QString ('0'+sendData[3]));

        /* Send a complete page */
        do {
            /* Fill a TKN Packet */
            for (int i=0; i<sizeof(TKN_Data); i++){
                sendData[i] = addr<=end? ((BYTE) h->getData(addr)): byteForEmpty;
                byteCount++;
                addr++;
            }

            /* Send it to tha MCU */
            while (TKN_PushData ((TKN_Data *) sendData, NODE_ID));

        } while ((addr % (PAGESIZE*2)!=0) );

    }

    QString msg = QString("Hex upload complete. Sent ") + QString::number(byteCount) + QString(" bytes.");
    emit consoleOut(msg);
}

