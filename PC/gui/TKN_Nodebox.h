#ifndef TKN_NODEBOX_H
#define TKN_NODEBOX_H

#include <QGroupBox>
#include <QPixmap>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>
#include <qwt_dial.h>

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
    void receiveData(TKN_Data*);

signals:
    void dataReceived(TKN_Data data);
    void consoleOut(QString msg);

private slots:
    void on_buttonSend_clicked();    
    void on_buttonRun_clicked();
    void on_buttonUpload_clicked();
    void on_buttonBrowseHex_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void voltometerSet(TKN_Data data);
    void dataEcho(TKN_Data data);

    void on_checkBox_log_toggled(bool checked);

private:
    Ui::TKN_NodeBox *ui;
    int PAGESIZE;
    int NODE_ID;
    QwtDial voltometer;
    QQueue<TKN_Data>dataQueue;
    QMutex dataQueueMutex;
    QSemaphore dataQueueSem;
    void flushRecDataQueue();
    bool dataDeque(TKN_Data *data, int timeout);
    void hexUpload();
};

#endif // TKN_NODEBOX_H
