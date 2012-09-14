#ifndef TKN_WINDOW_H
#define TKN_WINDOW_H

#include <QMainWindow>
#include <QList>
#include "TKN_Nodebox.h"

namespace Ui {
    class TKN_Window;
}

class TKN_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit TKN_Window(QWidget *parent = 0);
    ~TKN_Window();

    static void tokenReceivedStatic();
    static void dataReceivedStatic();

signals:
    void tokenReceived_signal(int);
    void dataReceived_signal();

private slots:
    void on_buttonStartStop_clicked();
    void on_actionAbout_triggered();
    void on_dataReceived();

private:
    Ui::TKN_Window *ui;

    void updateUI();
    QList<TKN_NodeBox*> nodeList;
    static TKN_Window *self;
    static const QString buttonStartText, buttonStopText;
    bool tknStarted;
    int tknCounter;
};

#endif
