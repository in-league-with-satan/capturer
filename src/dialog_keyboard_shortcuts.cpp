#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include <QStyle>
#include <QApplication>

#include "data_types.h"

#include "dialog_keyboard_shortcuts.h"

DialogKeyboardShortcuts::DialogKeyboardShortcuts(QWidget *parent)
    : QDialog(parent)
{
    QGridLayout *la_lines=new QGridLayout();

    row.resize(KeyCodeC::enm_size);

    for(int i=0; i<KeyCodeC::enm_size; ++i) {
        row[i]=new Row();

        la_lines->addWidget(row[i]->label=new QLabel(KeyCodeC::toString(i) + QLatin1Literal(":")), i, 0);
        la_lines->addWidget(row[i]->line_edit=new QLineEdit(QKeySequence(defaultQtKey(i)).toString()), i, 1);

        row[i]->line_edit->installEventFilter(this);
    }

    QPushButton *b_set_default=new QPushButton("reset to default");
    QPushButton *b_ok=new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogOkButton), "Ok");
    QPushButton *b_cancel=new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel");

    connect(b_set_default, SIGNAL(clicked(bool)), SLOT(toDefault()));
    connect(b_ok, SIGNAL(clicked(bool)), SLOT(accept()));
    connect(b_cancel, SIGNAL(clicked(bool)), SLOT(reject()));


    QHBoxLayout *la_ok_cancel=new QHBoxLayout();
    la_ok_cancel->addWidget(b_ok);
    la_ok_cancel->addWidget(b_cancel);

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addStretch(1);
    la_main->addLayout(la_lines);
    la_main->addWidget(b_set_default);
    la_main->addLayout(la_ok_cancel);
    la_main->addStretch(1);

    setLayout(la_main);
}

DialogKeyboardShortcuts::~DialogKeyboardShortcuts()
{
    for(int i=0; i<KeyCodeC::enm_size; ++i) {
        row[i]->label->deleteLater();
        row[i]->line_edit->deleteLater();

        delete row[i];
    }

    row.clear();
}

Qt::Key DialogKeyboardShortcuts::toQtKey(int code)
{
    if(code<0 || code>=KeyCodeC::enm_size)
        return Qt::Key_F1;

    QKeySequence seq(row[code]->line_edit->text());

    return seq.count()==1 ? (Qt::Key)seq[0] : Qt::Key_F1;
}

void DialogKeyboardShortcuts::setKey(int code, Qt::Key key)
{
    if(code<0 || code>=KeyCodeC::enm_size)
        return;

    row[code]->line_edit->setText(QKeySequence(key).toString());
}

Qt::Key DialogKeyboardShortcuts::defaultQtKey(int code)
{
    switch(code) {
    case KeyCodeC::About:
        return Qt::Key_F1;

    case KeyCodeC::FileBrowser:
        return Qt::Key_F2;

    case KeyCodeC::SmoothTransform:
        return Qt::Key_F3;

    case KeyCodeC::Rec:
        return Qt::Key_F4;

    case KeyCodeC::Info:
        return Qt::Key_F5;

    case KeyCodeC::RecState:
        return Qt::Key_F6;

    case KeyCodeC::Preview:
        return Qt::Key_F7;

    case KeyCodeC::PreviewFastYuv:
        return Qt::Key_F8;

    case KeyCodeC::PreviewCam:
        return Qt::Key_F9;

    case KeyCodeC::FullScreen:
        return Qt::Key_F11;

    case KeyCodeC::Exit:
        return Qt::Key_F12;

    case KeyCodeC::Menu:
        return Qt::Key_Menu;

    case KeyCodeC::Back:
        return Qt::Key_Backspace;

    case KeyCodeC::Enter:
        return Qt::Key_Return;

    case KeyCodeC::Up:
        return Qt::Key_Up;

    case KeyCodeC::Down:
        return Qt::Key_Down;

    case KeyCodeC::Left:
        return Qt::Key_Left;

    case KeyCodeC::Right:
        return Qt::Key_Right;
    }

    return Qt::Key_F1;
}

bool DialogKeyboardShortcuts::eventFilter(QObject *obj, QEvent *event)
{
    QLineEdit *line_edit=nullptr;

    for(int i=0; i<row.size(); ++i) {
        if(row[i]->line_edit==obj) {
            line_edit=row[i]->line_edit;
            break;
        }
    }

    if(line_edit) {
        if(event->type()==QEvent::KeyPress) {
            QKeyEvent *key_event=static_cast<QKeyEvent*>(event);

            if(key_event) {
                if(!key_event->modifiers()) {
                    line_edit->setText(QKeySequence(key_event->key()).toString());
                }

                key_event->ignore();

                return true;
            }
        }
    }

    return QDialog::eventFilter(obj, event);
}

void DialogKeyboardShortcuts::toDefault()
{
    for(int i=0; i<row.size(); ++i)
        row[i]->line_edit->setText(QKeySequence(defaultQtKey(i)).toString());
}

