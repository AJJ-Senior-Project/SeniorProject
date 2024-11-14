#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ndi_sender.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <windows.h>
#include <psapi.h>
#include <QStringList>
#include <QCameraDevice>

#include <iostream>  // Include for std::cerr, std::endl
#include <QDebug>

mainpage::mainpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainpage)
    , ndiReceiver(new NDIReceiver())
    , ndiSender(new NDISender())
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    // Set up the QGraphicsView
    ui->graphicsView->setScene(scene);

    // Populate the source combo boxes
    populateApplicationSources();
    populateCameraSources();
    populateAudioSources();

    // Connect the "Send" button to send signal slot
    connect(ui->pushButton_4, &QPushButton::clicked, this, &mainpage::on_sendSignalButton_clicked);

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
        ndiSender->stopAll();  // Ensure sender stops before destruction
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

    // Start discovering sources in a separate thread
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

void mainpage::on_pushButton_4_clicked()
{
    if (!ndiSender->initializeNDI()) {
        std::cerr << "Failed to initialize NDI sender." << std::endl;
        return;
    }

    ndiSender->sendMessage("Test message", 1, "GameName");
}

void mainpage::on_sourceComboBox_currentTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }

    qDebug() << "Starting to receive from source:" << text;

    // Stop any current receiving
    ndiReceiver->stopReceiving();

    // Start receiving from the selected source
    ndiReceiver->selectSource(text);
    ndiReceiver->startReceiving();
}

void mainpage::updateAvailableSources(const QStringList &sources)
{
    ui->sourceComboBox->clear();
    ui->sourceComboBox->addItems(sources);
}

void mainpage::displayVideoFrame(const QImage &frame)
{
    if (frame.isNull()) {
        qDebug() << "Received null image.";
        return;
    }

    if (!scene) {
        qDebug() << "Scene is null.";
        return;
    }

    scene->clear();
    scene->addPixmap(QPixmap::fromImage(frame));

    if (ui && ui->graphicsView) {
        ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    } else {
        qDebug() << "graphicsView is null.";
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
        ui->comboBox_2->addItem(camera.description(), QVariant::fromValue(camera));
    }
}

// Populate audio sources in combo box
void mainpage::populateAudioSources()
{
    auto audioInputs = QMediaDevices::audioInputs();  // List of available audio inputs
    for (const QAudioDevice &audio : audioInputs) {
        QString deviceName = audio.description();
        ui->comboBox->addItem(deviceName, QVariant::fromValue(audio));
    }
}

// Handle "Send" button click
void mainpage::on_sendSignalButton_clicked()
{
    QString selectedApplication = ui->comboBox_3->currentText();
    QString selectedCamera = ui->comboBox_2->currentText();
    QString selectedAudio = ui->comboBox->currentText();

    if (!ndiSender->initializeNDI()) {
        qDebug() << "Failed to initialize NDI sender.";
        return;
    }

    // Set message priority and game metadata
    ndiSender->sendMessage("Starting stream", 1, selectedApplication.toStdString());

    // Optionally send camera feed, audio, or both based on user selection
    if (!selectedCamera.isEmpty()) {
        qDebug() << "Starting camera feed from:" << selectedCamera;
        ndiSender->startCameraFeed(selectedCamera.toStdString());
    }

    if (!selectedAudio.isEmpty()) {
        qDebug() << "Starting audio feed from:" << selectedAudio;
        ndiSender->startAudioFeed(selectedAudio.toStdString());
    }

    qDebug() << "NDI stream started with selected options.";
}
