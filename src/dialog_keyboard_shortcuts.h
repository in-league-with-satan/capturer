#ifndef DIALOG_KEYBOARD_SHORTCUTS_H
#define DIALOG_KEYBOARD_SHORTCUTS_H

#include <QDialog>

class QLabel;
class QLineEdit;

class DialogKeyboardShortcuts : public QDialog
{
    Q_OBJECT

public:
    explicit DialogKeyboardShortcuts(QWidget *parent=0);
    ~DialogKeyboardShortcuts();

    Qt::Key toQtKey(int code);
    void setKey(int code, Qt::Key key);
    static Qt::Key defaultQtKey(int code);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void toDefault();

private:
    struct Row {
        QLabel *label;
        QLineEdit *line_edit;
    };

    QVector <Row*> row;
};

#endif // DIALOG_KEYBOARD_SHORTCUTS_H
