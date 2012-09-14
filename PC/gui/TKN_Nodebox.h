#ifndef TKN_NODEBOX_H
#define TKN_NODEBOX_H

#include <QGroupBox>

namespace Ui {
    class TKN_NodeBox;
}

class TKN_NodeBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit TKN_NodeBox(QWidget *parent, int id);
    ~TKN_NodeBox();

    void console_output(char *);

private slots:
    void on_buttonSend_clicked();

private:
    Ui::TKN_NodeBox *ui;

    int node_id;
};

#endif // TKN_NODEBOX_H
