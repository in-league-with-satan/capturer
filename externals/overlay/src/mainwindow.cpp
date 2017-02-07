#include <QDebug>
#include <QApplication>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QApplication::exit();
}

