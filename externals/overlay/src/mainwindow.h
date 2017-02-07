#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>

class QTimer;

class QmlMessenger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    QmlMessenger *messenger;


    QTimer *rec_progress_timer;
    QDateTime rec_progress_start_point;
    int rec_progress_size;

protected:
    virtual void closeEvent(QCloseEvent*);

private slots:
    void onRecStarted();
    void onRecProgressTimer();

};

#endif // MAINWINDOW_H
