#include "TKN_Window.h"
#include "ui_TKN_Window.h"
#include "TKN_Nodebox.h"
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QMap>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/TKN.h"
#include "lib/TKN_Util.h"

const QString TKN_Window::buttonStartText = QString("TKN Start");
const QString TKN_Window::buttonStopText = QString("TKN Stop");
const QString baudList[] = { "57600", "9600", "38400", "115200" };

TKN_Window* TKN_Window::self;

TKN_Window::TKN_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TKN_Window)
{
    ui->setupUi(this);
    ui->buttonStartStop->setText(buttonStartText);

    int i;

    /* available ports */
    char** port_list = listSerialPorts();
    for (i=0; *port_list; i++, port_list++){
        ui->comboBox_ComPort->insertItem(i, QString(*port_list));
    }

    /* baud rates */
    for (i=0; i<sizeof(baudList)/sizeof(baudList[0]) ; i++)
        ui->comboBox_Baud->insertItem(i, baudList[i]);

    /*Create the node list */
    nodeMap = QMap<int, TKN_NodeBox*>();

    this->tknStarted = false;
    this->updateUI();

    self = this;

    connect(this, SIGNAL(tokenReceived_signal(int)), ui->lcd_TokenCounter, SLOT(display(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(dataReceived_signal()), this, SLOT(on_dataReceived()), Qt::QueuedConnection);

    tknCounter = 0;
}

TKN_Window::~TKN_Window()
{
    delete ui;
}

void TKN_Window::on_actionAbout_triggered()
{
    QMessageBox::information(this, "About TKN Gui", "Chris Papapavlou\nchrispapapaulou@gmail.com", QMessageBox::Ok);
}

void TKN_Window::tokenReceivedStatic()
{
    emit self->tokenReceived_signal(self->tknCounter++);
}

void TKN_Window::dataReceivedStatic()
{
    emit self->dataReceived_signal();
}

void TKN_Window::updateUI()
{
    ui->comboBox_Baud->setEnabled(!tknStarted);
    ui->comboBox_ComPort->setEnabled(!tknStarted);
    ui->buttonStartStop->setText(tknStarted? buttonStopText : buttonStartText);
    ui->buttonStartStop->setStyleSheet(tknStarted? "* { background-color: rgba(200,0,0,255) }" : "* { background-color: rgba(0,200,0,255) }");

    QMap<int, TKN_NodeBox*>::iterator i;
    for (i = nodeMap.begin(); i != nodeMap.end(); ++i)
        (*i)->setEnabled(tknStarted);
}

void TKN_Window::on_buttonStartStop_clicked()
{
    if (!tknStarted)
    {
        int baud = ui->comboBox_Baud->currentText().toInt();
        int port = atoi(ui->comboBox_ComPort->currentText().toAscii().data()+3);

        if ( TKN_Init(port,
                      baud,
                      TKN_ID_DEFAULT,
                      TKN_Window::tokenReceivedStatic,
                      TKN_Window::dataReceivedStatic) )
            return;

        if (!nodeMap.empty()){
            qDeleteAll(nodeMap.begin(), nodeMap.end());
            nodeMap.clear();
        }

        for (BYTE *nodes = TKN_ListActiveNodes(); *nodes; nodes++){
            TKN_NodeBox *nd = new TKN_NodeBox(this, (int)(*nodes));
            nodeMap[*nodes] = nd;
            ui->centralWidget->layout()->addWidget(nd);

        }

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

void TKN_Window::on_dataReceived()
{
    char recData[sizeof(TKN_Data)+1];
    recData[sizeof(TKN_Data)]='\0';
    BYTE sender;

    if (sender=TKN_PopData((TKN_Data*) &recData)){
        if (nodeMap.contains((int)sender))
            nodeMap[(int)(sender)]->console_output(recData); //probably should make a slot to TKN_NodeBox...
    }

}
