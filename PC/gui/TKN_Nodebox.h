#ifndef TKN_NODEBOX_H
#define TKN_NODEBOX_H

#include <QGroupBox>
#include <QPixmap>
#include <QQueue>

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

public slots:
    void dataReceive(TKN_Data*);

private slots:
    void on_buttonSend_clicked();

    void consoleOut(TKN_Data *data);

    void on_buttonSendFile_clicked();

private:
    Ui::TKN_NodeBox *ui;

    int node_id;
    QQueue<TKN_Data>dataQueue;

};

#endif // TKN_NODEBOX_H
