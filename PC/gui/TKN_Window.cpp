#include "TKN_Window.h"
#include "ui_TKN_Window.h"
#include "TKN_Nodebox.h"
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QMap>
#include <QTimer>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/rs232.h"
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
    self = this;
    int i;

    /* available ports */
    char** port_list = listSerialPorts();
    for (i=0; *port_list; i++, port_list++)
        ui->comboBox_ComPort->insertItem(i, QString(*port_list));

    /* baud rates */
    for (i=0; i<sizeof(baudList)/sizeof(baudList[0]) ; i++)
        ui->comboBox_Baud->insertItem(i, baudList[i]);

    /*Create the node list */
    nodeMap = QMap<int, TKN_NodeBox*>();
    this->tknStarted = false;

    /* Signal connections */
    connect(this, SIGNAL(tokenReceived_signal(int)), ui->lcd_TokenCounter, SLOT(display(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(dataReceived()), this, SLOT(on_dataReceived()), Qt::QueuedConnection);

    this->updateUI();
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
    emit self->dataReceived();
}

void TKN_Window::updateUI()
{
    ui->comboBox_Baud->setEnabled(!tknStarted);
    ui->comboBox_ComPort->setEnabled(!tknStarted);
    ui->buttonStartStop->setText(tknStarted? buttonStopText : buttonStartText);
    ui->buttonStartStop->setStyleSheet(tknStarted? "* { background-color: rgba(200,0,0,255) }" : "* { background-color: rgba(0,200,0,255) }");
    shrink();
}

void TKN_Window::on_buttonStartStop_clicked()
{
    if (!tknStarted){
        this->startTkn();
    }
    else {
        this->stopTkn();
    }

    this->updateUI();
}

void TKN_Window::startTkn()
{
    int baud = ui->comboBox_Baud->currentText().toInt();

    /*Get the port index. Needed by rs232 lib */
    char *portName = ui->comboBox_ComPort->currentText().toAscii().data();
    int port = getPortIndexByName(portName);

    if (port<0)
        return;

    /* Init the network */
    if (TKN_Init(port, baud, TKN_ID_DEFAULT, TKN_Window::tokenReceivedStatic, TKN_Window::dataReceivedStatic))
        return;

    QString msg;
    msg = QString("Opened TKN in ")+ui->comboBox_ComPort->currentText();
    ui->textEditConsoleStatus->append(msg);
    msg = QString("Discovering nodes");
    ui->textEditConsoleStatus->append(msg);

    BYTE *nodes = TKN_ListActiveNodes(10);

    if (nodes == NULL){
        msg = QString("No connectivity");
        ui->textEditConsoleStatus->append(msg);
        TKN_Close();
        msg = QString("Closed TKN");
        ui->textEditConsoleStatus->append(msg);
        return;
    }

    while (*nodes){
        TKN_NodeBox *nd = new TKN_NodeBox(this, (int)(*nodes));
        nodeMap[*nodes] = nd;
        ui->centralWidget->layout()->addWidget(nd);
        nodes++;
    }

    if (TKN_Start())
        return;

    tknCounter = 0;
    this->tknStarted = true;
}

void TKN_Window::stopTkn()
{
    if ( !TKN_Stop()) {
        if (!nodeMap.empty()){
            QMap<int, TKN_NodeBox*>::iterator i;
            for (i = nodeMap.begin(); i != nodeMap.end(); ++i)
                ui->centralWidget->layout()->removeWidget(*i);

            qDeleteAll(nodeMap.begin(), nodeMap.end());
            nodeMap.clear();
        }

        TKN_Close();
        this->tknStarted = false;
        QTimer::singleShot(20, this, SLOT(shrink()));
    }
}

void TKN_Window::shrink()
{
   resize(0, 0);
   setMaximumSize(size());
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
