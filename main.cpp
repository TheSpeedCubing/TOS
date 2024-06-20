
#include <mainwindow.h>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QThread>
#include <QTimer>
#include <iostream>
#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <iostream>
#include "mainwindow.h"

using namespace std;
const int width = 280;
const int height = 450;
// MAIN
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(width, height);

    w.show();
    return a.exec();
}
