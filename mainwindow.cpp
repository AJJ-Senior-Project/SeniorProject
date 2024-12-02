#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <windows.h>
#include <psapi.h>
#include <QStringList>
#include <QCameraDevice>
#include <QDebug>
#include <QVBoxLayout>

mainpage::mainpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainpage)
    , ndiReceiver(new NDIReceiver())
    , ndiSender(new NDISender())
{
    ui->setupUi(this);

    // Set up the graphicsViews
    graphicsViews["graphicsView_3"] = ui->graphicsView_3;
    graphicsViews["graphicsView_4"] = ui->graphicsView_4;
    graphicsViews["graphicsView_5"] = ui->graphicsView_5;
    graphicsViews["graphicsView_6"] = ui->graphicsView_6;

    // Initialize scenes
    scenes["graphicsView_3"] = new QGraphicsScene(this);
    scenes["graphicsView_4"] = new QGraphicsScene(this);
    scenes["graphicsView_5"] = new QGraphicsScene(this);
    scenes["graphicsView_6"] = new QGraphicsScene(this);

    ui->graphicsView_3->setScene(scenes["graphicsView_3"]);
    ui->graphicsView_4->setScene(scenes["graphicsView_4"]);
    ui->graphicsView_5->setScene(scenes["graphicsView_5"]);
    ui->graphicsView_6->setScene(scenes["graphicsView_6"]);

    // Populate the source combo boxes
    populateApplicationSources();
    populateCameraSources();
    populateAudioSources();

    QVBoxLayout *senderDropdown = ui->verticalLayout_2;
    senderDropdown->setSpacing(1);
    senderDropdown->setContentsMargins(0, 0, 0, 0);

    // Connect signals from NDIReceiver to the UI slots
    connect(ndiReceiver, &NDIReceiver::sourcesDiscovered, this, &mainpage::updateAvailableSources);
    connect(ndiReceiver, &NDIReceiver::frameReceived, this, &mainpage::displayVideoFrame);
}

mainpage::~mainpage()
{
    if (ndiReceiver) {
        ndiReceiver->terminateReceiver();
        delete ndiReceiver;
        ndiReceiver = nullptr;
    }
    if (ndiSender) {
        ndiSender->stopAll();
        delete ndiSender;
        ndiSender = nullptr;
    }
    delete ui;
}

void mainpage::on_selectSendButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void mainpage::on_selectReceiveButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);

    // Initialize the NDI receiver
    if (!ndiReceiver->initializeReceiver()) {
        qDebug() << "Failed to initialize NDI Receiver.";
        return;
    }

    // Start discovering sources
    ndiReceiver->startDiscovery();
}

void mainpage::on_senderBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainpage::on_receiverBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainpage::on_sourceComboBox_currentTextChanged(const QString &text)
{
    // Deprecated; we're now displaying all sources
}

void mainpage::updateAvailableSources(const QStringList &sources)
{
    ui->sourceComboBox->clear();
    ui->sourceComboBox->addItems(sources);

    // Assign sources to graphicsViews
    int index = 0;
    QStringList graphicsViewNames = { "graphicsView_3", "graphicsView_4", "graphicsView_5", "graphicsView_6" };
    foreach (const QString &source, sources) {
        if (index < graphicsViewNames.size()) {
            QString viewName = graphicsViewNames[index];
            sourceScenes[source] = scenes[viewName];
            ndiReceiver->selectSource(source); // Automatically start receiving from this source
            ++index;
        } else {
            // No more graphicsViews available
            break;
        }
    }
}

void mainpage::displayVideoFrame(const QString &sourceName, const QImage &frame)
{
    if (frame.isNull()) {
        qDebug() << "Received null image from source:" << sourceName;
        return;
    }

    if (!sourceScenes.contains(sourceName)) {
        qDebug() << "No scene assigned for source:" << sourceName;
        return;
    }

    QGraphicsScene *scene = sourceScenes[sourceName];
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(frame));

    // Get the corresponding graphicsView
    foreach (const QString &viewName, scenes.keys()) {
        if (scenes[viewName] == scene) {
            QGraphicsView *view = graphicsViews[viewName];
            view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
            break;
        }
    }
}

// Populate application sources in combo box
void mainpage::populateApplicationSources()
{
    QStringList applications = getRunningApplications();
    ui->comboBox_3->addItems(applications);
}

QStringList mainpage::getRunningApplications()
{
    QStringList appList;

    // Using EnumWindows to get list of running applications
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL) {
            TCHAR windowTitle[MAX_PATH];
            GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));
            if (wcslen(windowTitle) > 0) {
                auto appList = reinterpret_cast<QStringList*>(lParam);
                appList->append(QString::fromWCharArray(windowTitle));
            }

        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&appList));

    return appList;
}

// Populate camera sources in combo box
void mainpage::populateCameraSources()
{
    auto cameras = QMediaDevices::videoInputs();  // List of available video cameras
    for (const auto &camera : cameras) {
        ui->comboBox_2->addItem(camera.description(), QVariant::fromValue(camera.id()));
    }
}

// Populate audio sources in combo box
void mainpage::populateAudioSources()
{
    auto audioInputs = QMediaDevices::audioInputs();  // List of available audio inputs
    for (const QAudioDevice &audio : audioInputs) {
        QString deviceName = audio.description();
        ui->comboBox->addItem(deviceName, QVariant::fromValue(audio.id()));
    }
}

// Handle "Send" button click
void mainpage::on_sendSignalButton_clicked()
{
    QString selectedApplication = ui->comboBox_3->currentText();
    QString selectedCameraId = ui->comboBox_2->currentData().toString();
    QString selectedAudioId = ui->comboBox->currentData().toString();

    if (!ndiSender->initializeNDI()) {
        qDebug() << "Failed to initialize NDI sender.";
        return;
    }

    ndiSender->sendMessage("Starting stream", 1, selectedApplication);

    // Start all streams
    ndiSender->startAllStreams(selectedCameraId, selectedAudioId, selectedApplication);

    qDebug() << "NDI streams started with selected options.";
}
