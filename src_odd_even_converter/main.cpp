#include <QCoreApplication>
#include <iostream>

#include "odd_even_converter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(a.arguments().size()<2)
        return 1;


    OddEvenConverterThread::Config cfg;

    foreach(QString arg, a.arguments()) {
        if(arg.startsWith("--src", Qt::CaseInsensitive))
            cfg.filename_src=arg.remove("--src=", Qt::CaseInsensitive).remove("\"").trimmed();

        if(arg.startsWith("--dst", Qt::CaseInsensitive))
            cfg.filename_dst=arg.remove("--dst=", Qt::CaseInsensitive).remove("\"").trimmed();

        if(arg.startsWith("--crf", Qt::CaseInsensitive))
            cfg.crf=arg.remove("--crf=", Qt::CaseInsensitive).trimmed().toInt();

        if(arg.startsWith("--swap_the_frame_order", Qt::CaseInsensitive))
            cfg.swap_the_frame_order=true;
    }


    OddEvenConverterThread conv;

    QObject::connect(&conv, SIGNAL(finished()), &a, SLOT(quit()));


    conv.start(cfg);


    return a.exec();
}
