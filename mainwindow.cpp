#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

