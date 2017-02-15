#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>

class QTimer;

class QmlMessenger;
class OverlayView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    QmlMessenger *messenger;
    OverlayView *overlay_view;

    QTimer *rec_progress_timer;
    QDateTime rec_progress_start_point;
    int rec_progress_size;

protected:
    virtual void closeEvent(QCloseEvent*);

private slots:
    void onRecStarted();
    void onRecProgressTimer();

    void keyUp();
    void keyDown();
    void keyLeft();
    void keyRight();

};

#endif // MAINWINDOW_H
