#ifndef TKN_WINDOW_H
#define TKN_WINDOW_H

#include <QMainWindow>
#include <QMap>
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
    void dataReceived();

private slots:
    void on_buttonStartStop_clicked();
    void on_actionAbout_triggered();
    void shrink();
    void on_dataReceived();

private:
    Ui::TKN_Window *ui;
    QMap<int, TKN_NodeBox*> nodeMap;
    static TKN_Window *self;
    static const QString buttonStartText, buttonStopText;
    bool mTknStarted;
    int mTknCounter;
    QTime *mTime;

    void updateUI();
    void startTkn();
    void stopTkn();

};

#endif
