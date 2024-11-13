#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ndi_sender.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <windows.h>
#include <psapi.h>
#include <QStringList>

#include <iostream>       // Include for std::cerr, std::endl
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

    // Connect the sourceComboBox signal

}


mainpage::~mainpage()
{
    if (ndiReceiver) {
        ndiReceiver->terminateReceiver();
        delete ndiReceiver;
        ndiReceiver = nullptr;
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
    static NDISender sender;
    if (!sender.initializeNDI()) {
        std::cerr << "Failed to initialize NDI sender." << std::endl;
        return;
    }

    sender.sendMessage("Test message", 1, "GameName");
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
        QString deviceName = audio.description();  // Some Qt versions use `description()` for device name
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

    // Set message priority (e.g., 1) and game metadata
    ndiSender->sendMessage("Starting stream", 1, selectedApplication.toStdString());

    // Optionally send camera feed, audio, or both based on user selection
    if (!selectedCamera.isEmpty()) {
        qDebug() << "Sending camera feed from:" << selectedCamera;
        ndiSender->sendCameraFeed(selectedCamera.toStdString());
    }

    if (!selectedAudio.isEmpty()) {
        qDebug() << "Sending audio feed from:" << selectedAudio;
        ndiSender->sendAudio(selectedAudio.toStdString());
    }

    qDebug() << "NDI stream started with selected options.";
}

QStringList mainpage::getRunningApplications() {
    QStringList appList;

    DWORD processes[1024], processCount, cbNeeded;
    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        return appList;
    }

    processCount = cbNeeded / sizeof(DWORD);
    for (unsigned int i = 0; i < processCount; ++i) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess) {
                HMODULE hMod;
                DWORD cbNeeded;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                    TCHAR processName[MAX_PATH];
                    if (GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR))) {
                        appList << QString::fromWCharArray(processName);
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    return appList;
}
