#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include "ndi_receiver.h"  // Include the NDIReceiver header

QT_BEGIN_NAMESPACE
namespace Ui {
class mainpage;
}
QT_END_NAMESPACE

class mainpage : public QWidget
{
    Q_OBJECT

public:
    mainpage(QWidget *parent = nullptr);
    ~mainpage();

private slots:
    void on_selectSendButton_clicked();
    void on_selectReceiveButton_clicked();
    void on_senderBackButton_clicked();
    void on_receiverBackButton_clicked();
    void on_pushButton_4_clicked();

private:
    Ui::mainpage *ui;
    NDIReceiver *ndiReceiver;  // Add a member pointer for NDIReceiver
};

#endif // MAINPAGE_H
