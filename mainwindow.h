#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QComboBox>
#include "ndi_sender.h"
#include "ndi_receiver.h"

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

    QStringList getRunningApplications();

private slots:
private slots:
    void on_selectSendButton_clicked();
    void on_selectReceiveButton_clicked();
    void on_senderBackButton_clicked();
    void on_receiverBackButton_clicked();
    void on_pushButton_4_clicked();
    void on_sourceComboBox_currentTextChanged(const QString &text);
    void on_sendSignalButton_clicked();

    void populateApplicationSources();
    void populateCameraSources();
    void populateAudioSources();

    void updateAvailableSources(const QStringList &sources);
    void displayVideoFrame(const QImage &frame);



private:
    Ui::mainpage *ui;
    NDIReceiver *ndiReceiver;
    NDISender *ndiSender;
    QGraphicsScene *scene;
};

#endif // MAINPAGE_H
