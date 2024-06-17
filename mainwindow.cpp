#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QWidget"
#include <QMouseEvent>
#include "main.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::mousePressEvent(QMouseEvent *event) {
    handleMousePressed(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    handleMouseReleased(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    handleMouseMoved(event);
}
