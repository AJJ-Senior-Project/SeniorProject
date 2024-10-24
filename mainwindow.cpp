#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ndi_sender.h"
#include "ndi_receiver.h"

mainpage::mainpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainpage)
{
    ui->setupUi(this);
}

mainpage::~mainpage()
{
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
    qDebug("yes we swapped up");
    // Create an NDIReceiver instance locally
    NDIReceiver ndiReceiver;

    // Initialize the NDI receiver
    if (!ndiReceiver.initializeReceiver()) {
        qDebug("Failed to initialize NDI Receiver.");
        return;
    }

    // Discover available NDI sources
    ndiReceiver.discoverSources();

    // Select a specific source (update this with the actual name of your NDI sender)
    std::string selectedSourceName = "NDI Sender"; // Replace with the correct name
    if (ndiReceiver.selectSource(selectedSourceName)) {
        qDebug("Successfully selected the NDI source: %s", selectedSourceName.c_str());

        // Receive video and metadata (non-blocking call if it's okay to do it here)
        ndiReceiver.receiveVideoAndMetadata(); // This will block, so consider threading if needed
    } else {
        qDebug("Failed to select NDI source.");
    }
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
