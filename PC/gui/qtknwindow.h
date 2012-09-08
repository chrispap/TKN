#ifndef QTKNWINDOW_H
#define QTKNWINDOW_H

#include <QMainWindow>

namespace Ui {
    class QTKNWindow;
}

class QTKNWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QTKNWindow(QWidget *parent = 0);
    ~QTKNWindow();

private slots:
    void on_buttonStartStop_clicked();

    void on_actionAbout_triggered();

    void on_button_Send_clicked();

    void on_buttonRec_clicked();

    void on_buttonClear_clicked();

private:
    Ui::QTKNWindow *ui;

    void updateUI();

    static const QString buttonStartText, buttonStopText;

    bool tknStarted;

};

#endif // QTKNWINDOW_H
