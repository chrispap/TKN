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
    int getNode_id(){return node_id;}

public slots:
    void dataReceive(TKN_Data*);

signals:
    void dataReceived();

private slots:
    void on_buttonSend_clicked();
    void on_buttonSendFile_clicked();
    void on_dataReceived();
    void consoleOut(TKN_Data *data);

private:
    Ui::TKN_NodeBox *ui;

    int node_id;
    QQueue<TKN_Data>dataQueue;
    QMutex dataQueueMutex;
    QSemaphore dataQueueSem;

    void sendFile();
};

#endif // TKN_NODEBOX_H
