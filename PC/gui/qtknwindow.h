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
    void on_sendButton_clicked();

    void on_buttonInit_clicked();

private:
    Ui::QTKNWindow *ui;
};

#endif // QTKNWINDOW_H
