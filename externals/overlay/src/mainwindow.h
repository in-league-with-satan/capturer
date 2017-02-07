#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:

protected:
    virtual void closeEvent(QCloseEvent*);

private slots:

};

#endif // MAINWINDOW_H
