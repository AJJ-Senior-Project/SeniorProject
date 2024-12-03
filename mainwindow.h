#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QComboBox>
#include <QMap>
#include <QMutex>
#include "ndi_sender.h"
#include "ndi_receiver.h"
#include <Processing.NDI.Lib.h>

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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_selectSendButton_clicked();
    void on_selectReceiveButton_clicked();
    void on_senderBackButton_clicked();
    void on_receiverBackButton_clicked();
    void on_sourceComboBox_currentTextChanged(const QString &text);
    void on_sendSignalButton_clicked();

    void populateApplicationSources();
    void populateCameraSources();
    void populateAudioSources();

    void updateAvailableSources(const QStringList &sources);
    void displayVideoFrame(const QString &sourceName, const QImage &frame);

    // New slots for handling frame selection and combining
    void onGraphicsViewClicked(const QString &sourceName);
    void startCombiningFrames();
    void combineAndSendFrames();

    // New methods to update highlights
    void updateHighlights();

private:
    Ui::mainpage *ui;
    NDIReceiver *ndiReceiver;
    NDISender *ndiSender;
    QMap<QString, QGraphicsScene*> scenes;
    QMap<QString, QGraphicsView*> graphicsViews;
    QMap<QString, QGraphicsScene*> sourceScenes;

    // Member variables for selected sources and frames
    QString primarySourceName;
    QString secondarySourceName;
    QImage primaryFrame;
    QImage secondaryFrame;

    NDIlib_send_instance_t ndiSendCombined;
    QTimer *combineAndSendTimer;

    // Map from QGraphicsView to source name
    QMap<QGraphicsView*, QString> viewToSourceMap;

    QMutex frameMutex;

    QGraphicsScene *previewScene;  // Scene for graphicsView_2
};

#endif // MAINPAGE_H
