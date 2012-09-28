#ifndef TKN_NODEBOX_H
#define TKN_NODEBOX_H

#include <QGroupBox>
#include <QPixmap>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "lib/TKN.h"

namespace Ui {
    class TKN_NodeBox;
}

class TKN_NodeBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit TKN_NodeBox(QWidget *parent, int id);
    ~TKN_NodeBox();
    int getNode_id(){return NODE_ID;}

public slots:
    void dataReceive(TKN_Data*);

signals:
    void dataReceived();
    void consoleOut(QString msg);

private slots:
    void on_buttonSend_clicked();
    void on_buttonHexUpload_clicked();
    //void on_dataReceived();
    void on_buttonBrowseHex_clicked();
    void receivedDataEcho(TKN_Data *data);

private:
    Ui::TKN_NodeBox *ui;
    QQueue<TKN_Data>dataQueue;
    QMutex dataQueueMutex;
    QSemaphore dataQueueSem;

    int PAGESIZE;
    int NODE_ID;

    void flushRecDataQueue();
    void dataDeque(TKN_Data *data);
    void hexUpload();
};

#endif // TKN_NODEBOX_H
