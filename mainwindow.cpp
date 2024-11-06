#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ndi_sender.h"
#include "ndi_receiver.h"

mainpage::mainpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainpage)
    , ndiReceiver(new NDIReceiver())  // Create a member instance of NDIReceiver
{
    ui->setupUi(this);
}

mainpage::~mainpage()
{
    // Clean up the NDI receiver when mainpage is destroyed
    if (ndiReceiver) {
        ndiReceiver->terminateReceiver();  // Ensure threads and resources are cleaned up
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
    // Switch to the new page in the stacked widget
    ui->stackedWidget->setCurrentIndex(2);
    qDebug("Switched to Receive Page");

    // Initialize the NDI receiver if not already initialized
    if (!ndiReceiver->initializeReceiver()) {
        qDebug("Failed to initialize NDI Receiver.");
        return;
    }

    // Discover available NDI sources
    ndiReceiver->discoverSources();

     ndiReceiver->startReceiving();

}

void mainpage::on_senderBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainpage::on_receiverBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

// Handle the "Send" button click
void mainpage::on_pushButton_4_clicked()
{
    static NDISender sender;  // Ensure only one instance is created and reused
    if (!sender.initializeNDI()) {
        std::cerr << "Failed to initialize NDI sender." << std::endl;
        return;
    }

    // Example of sending a message
    sender.sendMessage("Test message", 1, "GameName");

    // You can trigger other sender functionalities here, e.g., send video/audio.
}
