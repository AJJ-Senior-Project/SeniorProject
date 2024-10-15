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
