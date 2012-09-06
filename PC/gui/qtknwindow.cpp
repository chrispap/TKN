#include "qtknwindow.h"
#include "ui_qtknwindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/TKN.h"
#include "lib/TKN_Util.h"


QTKNWindow::QTKNWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QTKNWindow)
{
    ui->setupUi(this);

}

QTKNWindow::~QTKNWindow()
{
    delete ui;
}

void QTKNWindow::on_buttonInit_clicked()
{
    int port = ui->lineEdit_ComPort->text().toInt();
    int baud = ui->lineEdit_Baud->text().toInt();

    if ( TKN_Init(port, baud, TKN_ID_DEFAULT))
        return;
    if ( TKN_Start())
        return;

    ui->buttonInit->setEnabled(false);
    ui->lineEdit_Baud->setEnabled(false);
    ui->lineEdit_ComPort->setEnabled(false);
    ui->sendButton->setEnabled(true);
}

void QTKNWindow::on_sendButton_clicked()
{
    int dest_id = ui->lineEdit_DestId->text().toInt();
    TKN_Data data;
    qMemSet(&data, 0, sizeof(data));
    QByteArray bytes = ui->lineEdit_DataToSend->text().toAscii();
    memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    TKN_PushData(&data, dest_id);
}

