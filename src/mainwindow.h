#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

class QComboBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QTextEdit;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    void init();

    void stopProc();

    void load();
    void save();

    QComboBox *cb_device;
    QComboBox *cb_mode;

    QLineEdit *le_quality;

    QLabel *l_status;

    QTextEdit *te_out;

    QPushButton *b_start_stop;

    QCheckBox *cb_restart_rec_on_drop_frames;

    QProcess proc;

private slots:
    void onDeviceChanged(const QString &device);

    void startStop();
    void readProcStdOutput();
    void readProcErrOutput();
    void stateChanged(const QProcess::ProcessState &state);
};

#endif // MAINWINDOW_H
