#include "qtknwindow.h"
#include "ui_qtknwindow.h"
#include <QMessageBox>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/TKN.h"
#include "lib/TKN_Util.h"

const QString TKNWindow::buttonStartText = QString("TKN Start");
const QString TKNWindow::buttonStopText = QString("TKN Stop");
TKNWindow* TKNWindow::self;

TKNWindow::TKNWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QTKNWindow)
{
    ui->setupUi(this);
    ui->buttonStartStop->setText(buttonStartText);

    for (int i=0; i<5; i++)
        ui->comboBox_destId->insertItem(i, QString('1'+i));

    this->tknStarted = false;
    this->updateUI();

    self = this;
}

TKNWindow::~TKNWindow()
{
    delete ui;
}

void TKNWindow::tokenReceived()
{
    self->ui->lcd_TokenCounter->display( self->ui->lcd_TokenCounter->value() + 1);

}

void TKNWindow::dataReceived()
{
    self->on_buttonRec_clicked();
}

void TKNWindow::updateUI()
{
    ui->lineEdit_Baud->setEnabled(!tknStarted);
    ui->lineEdit_ComPort->setEnabled(!tknStarted);
    ui->groupBox_Send->setEnabled(tknStarted);
    ui->buttonRec->setEnabled(tknStarted);
    ui->buttonStartStop->setText(tknStarted? buttonStopText : buttonStartText);
    ui->buttonStartStop->setStyleSheet(tknStarted? "* { background-color: rgba(200,0,0,255) }" : "* { background-color: rgba(0,200,0,255) }");
}

void TKNWindow::on_buttonStartStop_clicked()
{
    if (!tknStarted) {
        int port = ui->lineEdit_ComPort->text().toInt();
        int baud = ui->lineEdit_Baud->text().toInt();

        if ( TKN_Init(port, baud, TKN_ID_DEFAULT, TKNWindow::tokenReceived, TKNWindow::dataReceived))
            return;
        if ( TKN_Start())
            return;

        this->tknStarted = true;
        this->updateUI();
    }
    else {
        if ( !TKN_Stop()) {
            TKN_Close();
            this->tknStarted = false;
            this->updateUI();
        }
    }

}

void TKNWindow::on_button_Send_clicked()
{
    int dest_id = ui->comboBox_destId->currentText().toInt();
    TKN_Data data;
    qMemSet(&data, 0, sizeof(data));
    QByteArray bytes = ui->lineEdit_DataToSend->text().toAscii();
    memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    TKN_PushData(&data, dest_id);
}

void TKNWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, "About", "About...", QMessageBox::Ok);
}

void TKNWindow::on_buttonRec_clicked()
{
    char recd[sizeof(TKN_Data)+1];
    recd[sizeof(TKN_Data)]=0;

    if (TKN_PopData((TKN_Data*)(&recd)) >= 0) {
        ui->textEdit_Console->append( QString(recd));
    }


}

void TKNWindow::on_buttonClear_clicked()
{
    ui->textEdit_Console->clear();
}
