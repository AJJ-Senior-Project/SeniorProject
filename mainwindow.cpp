#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ndi_sender.h"

#include <iostream>       // Include for std::cerr, std::endl
#include <QDebug>
#include <QVBoxLayout>

mainpage::mainpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainpage)
    , ndiReceiver(new NDIReceiver())
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    // Set up the QGraphicsView
    ui->graphicsView->setScene(scene);

    QVBoxLayout *senderDropdown = ui->verticalLayout_10;
    senderDropdown->setSpacing(1);
    senderDropdown->setContentsMargins(0, 0, 0, 0);

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



