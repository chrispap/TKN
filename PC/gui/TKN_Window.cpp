#include "TKN_Window.h"
#include "ui_TKN_Window.h"
#include "TKN_Nodebox.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QMap>

#include "lib/rs232.h"
#include "lib/TKN.h"
#include "lib/TKN_Util.h"

TKN_Window* TKN_Window::self;
const QString TKN_Window::buttonStartText = QString("TKN Start");
const QString TKN_Window::buttonStopText = QString("TKN Stop");
const QString TKN_Window::baudList[] = { "57600", "9600", "38400", "115200" };


TKN_Window::TKN_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TKN_Window)
{
    /* Ui setup */
    ui->setupUi(this);
    ui->buttonStartStop->setText(buttonStartText);
    self = this;
    mTime = new QTime();
    int i;

    /* available ports */
    char** port_list = listSerialPorts();
    if (port_list){
        for (i=0; *port_list; i++, port_list++)
            ui->comboBox_ComPort->insertItem(i, QString(*port_list));
    }

    /* baud rates */
    for (i=0; i<sizeof(baudList)/sizeof(baudList[0]) ; i++)
        ui->comboBox_Baud->insertItem(i, baudList[i]);

    this->mTknStarted = false;

    /* Signal connections */
    connect(this, SIGNAL(tokenReceived(int)), ui->lcd_TokenCounter, SLOT(display(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(dataReady()), this, SLOT(dataReceive()), Qt::QueuedConnection);

    this->ui->labelAVR->setPixmap( QPixmap(":/AVR_Chip-W180px.png"));
    this->updateUI();
}

TKN_Window::~TKN_Window()
{
    delete ui;
}

void TKN_Window::closeEvent(QCloseEvent *evt){
    if (nodeMap.isEmpty())
        evt->accept();
    else {
        emit consoleOut("Stop the network first.");
        evt->ignore();
    }
}

void TKN_Window::tokenReceivedCallback()
{
    static const int tknInv=50;
    self->mTknCounter++;

    //emit self->tokenReceived(self->mTknCounter++);
    //return;

    /* Every tknInv tokens show the rate
       of the respective time period */

    //emit self->tokenReceived(self->mTknCounter); return;

    if (self->mTknCounter%tknInv == 0) {
        emit self->tokenReceived(1000*tknInv/self->mTime->elapsed());
        self->mTime->start();
    }
}

void TKN_Window::dataReceivedCallback()
{
    /* In this point just emit the signal
       so that the reception will take place
       in the QT ui thread and not in here (TKN Thread) */
    emit self->dataReady();
}

void TKN_Window::dataReceive()
{
    TKN_Data recData;
    int sender;

    while ((sender=TKN_PopData(&recData)) != -1) {
        if (nodeMap.contains(sender)) {
            nodeMap[(int)(sender)]->dataReceive(&recData);
        }
    }

}

void TKN_Window::on_buttonStartStop_clicked()
{
    if (!mTknStarted){
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
    char portName[32];
    strcpy(portName, ui->comboBox_ComPort->currentText().toAscii().data());
    int port = getPortIndexByName(portName);

    if (port<0){
        consoleOut("Not a valid port");
        return;
    }

    /* Init the network */
    if (TKN_Init(port, baud, TKN_ID_DEFAULT, TKN_Window::tokenReceivedCallback, TKN_Window::dataReceivedCallback))
        return;

    QString msg;
    msg = QString("Opened TKN in ")+ui->comboBox_ComPort->currentText();
    consoleOut(msg);
    msg = QString("Discovering nodes");
    consoleOut(msg);

    BYTE *nodes = TKN_ListActiveNodes(10);

    if (nodes == NULL){
        msg = QString("No connectivity");
        consoleOut(msg);
        TKN_Close();
        msg = QString("Closed TKN");
        consoleOut(msg);
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

    TKN_PrintCols();

    mTknCounter = 0;
    this->mTknStarted = true;
    mTime->start();
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
        this->mTknStarted = false;
        QTimer::singleShot(20, this, SLOT(shrink()));
    }
    else emit consoleOut("Network is unstopable. (Not responding)");
}

void TKN_Window::shrink()
{
   resize(0, height());
}

void TKN_Window::on_actionAbout_triggered()
{
    QMessageBox::information(this, "About TKN Gui", "Chris Papapavlou\nchrispapapaulou@gmail.com", QMessageBox::Ok);
}

void TKN_Window::updateUI()
{
    ui->comboBox_Baud->setEnabled(!mTknStarted);
    ui->comboBox_ComPort->setEnabled(!mTknStarted);
    ui->buttonStartStop->setText(mTknStarted? buttonStopText : buttonStartText);
    ui->buttonStartStop->setStyleSheet(mTknStarted? "* { background-color: rgba(200,0,0,255) }" : "* { background-color: rgba(0,200,0,255) }");
    shrink();
}

void TKN_Window::consoleOut(QString msg)
{
    ui->textEditConsoleStatus->append(msg);
}

