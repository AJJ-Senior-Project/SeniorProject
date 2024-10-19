#ifndef mainpage_H
#define mainpage_H

#include <QWidget>

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

private:
    Ui::mainpage *ui;
};
#endif // mainpage_H
