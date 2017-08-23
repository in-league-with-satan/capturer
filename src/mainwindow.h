#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QVariantMap>

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
    QComboBox *cb_input_format;

    QComboBox *cb_encoder;
    QComboBox *cb_preset;
    QComboBox *cb_pixel_format;

    QLineEdit *le_quality;

    QLabel *l_status;

    QTextEdit *te_out;

    QPushButton *b_start_stop;

    QCheckBox *cb_restart_rec_on_drop_frames;

    QCheckBox *cb_half_fps;

    QProcess proc;

    QVariantMap map_pixel_format;
    QVariantMap map_preset;

private slots:
    void onDeviceChanged(const QString &device);
    void onEncoderChanged(int index);
    void onPresetChanged(int index);
    void onPixelFormatChanged(int index);

    void startStop();
    void readProcStdOutput();
    void readProcErrOutput();
    void stateChanged(const QProcess::ProcessState &state);
};

#endif // MAINWINDOW_H
