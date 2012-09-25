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

    void consoleOut(QString msg);

    static void tokenReceivedCallback();
    static void dataReceivedCallback();

signals:
    void tokenReceived(int);
    void dataReady();

private slots:
    void on_buttonStartStop_clicked();
    void on_actionAbout_triggered();

    void shrink();
    void dataReceive();

private:
    Ui::TKN_Window *ui;

    static TKN_Window *self;
    static const QString buttonStartText, buttonStopText;
    static const QString baudList[];

    bool mTknStarted;
    int mTknCounter;
    QMap<int, TKN_NodeBox*> nodeMap;
    QTime *mTime;

    void updateUI();
    void startTkn();
    void stopTkn();

};

#endif
