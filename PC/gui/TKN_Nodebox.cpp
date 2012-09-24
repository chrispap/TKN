#include "TKN_Nodebox.h"
#include "ui_TKN_Nodebox.h"
#include <QPixmap>
#include <QDebug>
#include <QThread>
#include <QtConcurrentRun>

#include "lib/TKN.h"


void sendFile(BYTE dest_id)
{
    int packC=0;
    int r;

    do{
        r = TKN_PushData ((TKN_Data *) "__From Laptop___", dest_id);
        if (!r)
            packC++;
    }while (packC<1000);

}

TKN_NodeBox::TKN_NodeBox(QWidget *parent, int id) :
    QGroupBox(parent),
    ui(new Ui::TKN_NodeBox)
{
    ui->setupUi(this);

    this->node_id = id;
    this->setTitle(QString("Node ").append(QString('0'+node_id)));

    QPixmap avrChip = QPixmap(":/AVR_Chip-W180px.png");
    this->ui->labelAVR->setPixmap( avrChip);

}

TKN_NodeBox::~TKN_NodeBox()
{
    delete ui;
}

void TKN_NodeBox::dataReceive(TKN_Data *data)
{
//    qDebug() << "Line: " << __LINE__ << " - " << QThread::currentThreadId();

    consoleOut(data);
}

void TKN_NodeBox::consoleOut(TKN_Data *data)
{
//    qDebug() << "Line: " << __LINE__ << " - " << QThread::currentThreadId();
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
    QtConcurrent::run(sendFile, node_id);
}
