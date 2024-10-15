#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>  // If you're using QImage for the frame

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ndiSender(new NDISender())  // Initialize NDISender
{
    ui->setupUi(this);

    // Connect the button to the sendFrame method in NDISender
    connect(ui->SendButton, &QPushButton::clicked, this, [=]() {
        QImage image(1920, 1080, QImage::Format_RGB32);  // Create a test frame
        ndiSender->sendFrame(image);  // Call the sendFrame method
    });
}

MainWindow::~MainWindow() {
    delete ui;
    delete ndiSender;  // Clean up NDISender
}
