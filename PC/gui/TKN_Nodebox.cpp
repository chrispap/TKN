#include "TKN_Nodebox.h"
#include "ui_TKN_Nodebox.h"

#include "../lib/TKN.h"

TKN_NodeBox::TKN_NodeBox(QWidget *parent, int id) :
    QGroupBox(parent),
    ui(new Ui::TKN_NodeBox)
{
    ui->setupUi(this);

    this->node_id = id;
    this->setTitle(QString("Node #").append(QString('0'+node_id)));
}

TKN_NodeBox::~TKN_NodeBox()
{
    delete ui;
}

void TKN_NodeBox::on_buttonSend_clicked()
{
    TKN_Data data;
    qMemSet(&data, 0, sizeof(data));
    QByteArray bytes = ui->lineEdit_DataToSend->text().toAscii();
    memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    TKN_PushData(&data, node_id);
}

void TKN_NodeBox::console_output(char *data)
{
    ui->textEditConsole->append(data);
}
