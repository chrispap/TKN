#include "qtknwindow.h"
#include "ui_qtknwindow.h"
#include <QMessageBox>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/TKN.h"
#include "lib/TKN_Util.h"

const QString QTKNWindow::buttonStartText = QString("TKN Start");
const QString QTKNWindow::buttonStopText = QString("TKN Stop");

QTKNWindow::QTKNWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QTKNWindow)
{
    ui->setupUi(this);
    ui->buttonStartStop->setText(buttonStartText);

    for (int i=0; i<5; i++)
        ui->comboBox_destId->insertItem(i, QString('1'+i));

    this->tknStarted = false;
    this->updateUI();

}

QTKNWindow::~QTKNWindow()
{
    delete ui;
}

void QTKNWindow::updateUI()
{
    ui->lineEdit_Baud->setEnabled(!tknStarted);
    ui->lineEdit_ComPort->setEnabled(!tknStarted);
    ui->groupBox_Send->setEnabled(tknStarted);
    ui->buttonRec->setEnabled(tknStarted);
    ui->buttonStartStop->setText(tknStarted? buttonStopText : buttonStartText);
    ui->buttonStartStop->setStyleSheet(tknStarted? "* { background-color: rgba(200,0,0,255) }" : "* { background-color: rgba(0,200,0,255) }");
}

void QTKNWindow::on_buttonStartStop_clicked()
{
    if (!tknStarted) {
        int port = ui->lineEdit_ComPort->text().toInt();
        int baud = ui->lineEdit_Baud->text().toInt();

        if ( TKN_Init(port, baud, TKN_ID_DEFAULT))
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

void QTKNWindow::on_button_Send_clicked()
{
    int dest_id = ui->comboBox_destId->currentText().toInt();
    TKN_Data data;
    qMemSet(&data, 0, sizeof(data));
    QByteArray bytes = ui->lineEdit_DataToSend->text().toAscii();
    memcpy(&data, bytes.data(), qMin(TKN_DATA_SIZE, bytes.size()));
    TKN_PushData(&data, dest_id);
}

void QTKNWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, "About", "About...", QMessageBox::Ok);
}

void QTKNWindow::on_buttonRec_clicked()
{
    char recd[sizeof(TKN_Data)+1];
    recd[sizeof(TKN_Data)]=0;

    if (TKN_PopData((TKN_Data*)(&recd)) >= 0) {
        ui->textEdit_Console->append( QString(recd));
    }


}

void QTKNWindow::on_buttonClear_clicked()
{
    ui->textEdit_Console->clear();
}
