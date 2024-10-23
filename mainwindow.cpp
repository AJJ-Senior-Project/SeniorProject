#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ndi_sender.h"

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
    ui->stackedWidget->setCurrentIndex(2);
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
