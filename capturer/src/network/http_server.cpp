/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QBuffer>
#include <QPixmap>
#include <QIcon>

#include <assert.h>

#ifdef LIB_QHTTP

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

using namespace qhttp::server;

#endif

#include "settings.h"
#include "settings_model.h"

#include "http_server.h"

class TableBuilder
{
public:
    QString open(int col_count, QString style="", QString class_str="", QString custom_str="", int border=0, int cellpadding=0, int cellspacing=0) {
        this->col_count=col_count;
        QString add_params;

        if(!style.isEmpty())
            add_params=QString(" style='%1'").arg(style);

        if(!class_str.isEmpty())
            add_params+=QString(" class='%1'").arg(class_str);

        if(!custom_str.isEmpty())
            add_params+=" " + custom_str;

        return QString("<table%1 border=%2 cellpadding=%3 cellspacing=%4 %5>\n").arg(add_params).arg(border).arg(cellpadding).arg(cellspacing);
    }

    QString header(const QStringList &field) {
        assert(field.size()==col_count);

        QString res;

        foreach(const QString &val, field) {
            res+=QString("<b><center>%1</center></b>").arg(val);
        }

        return res;
    }

    QString row(const QStringList &field, const QStringList &atrib=QStringList()) {
        QString res=QStringLiteral("  <tr>\n");

        if(atrib.isEmpty()) {
            foreach(const QString &val, field) {
                res+=QString("    <td>%1</td>\n").arg(val);
            }

        } else {
            assert(field.size()==atrib.size());

            for(int i=0; i<field.size(); ++i) {
                res+=QString("    <td %1>%2</td>\n").arg(atrib.at(i)).arg(field.at(i));
            }
        }

        res+=QStringLiteral("  </tr>\n");

        return res;
    }

    QString close() {
        return QStringLiteral("</table>\n");
    }

private:
    int col_count;
};

QMap <QString, QString> queryListToMap(QList <QPair <QString, QString> > list)
{
    QMap <QString, QString> map;

    foreach(auto pair, list)
        map.insert(pair.first, pair.second);

    return map;
}

HttpServer::HttpServer(quint16 port, QObject *parent)
    : QObject(parent)
    , settings_model(nullptr)
{
    if(port<1)
        return;

#ifdef LIB_QHTTP

    QHttpServer *server=new QHttpServer(this);

    last_buf_update=0;

    ((QHttpServer*)server)->listen(QHostAddress::Any, port,
                                   [&](QHttpRequest *req, QHttpResponse *res) {
        if(req->method()==qhttp::THttpMethod::EHTTP_GET) {
            res->setStatusCode(qhttp::ESTATUS_OK);

            QUrlQuery url=QUrlQuery(req->url());


            if(url.hasQueryItem(QStringLiteral("key_code"))) {
                emit keyPressed(url.queryItemValue(QStringLiteral("key_code")).toInt());
            }

            if(url.hasQueryItem(QStringLiteral("player_seek"))) {
                emit playerSeek(url.queryItemValue(QStringLiteral("player_seek")).toLongLong());
            }

            const QString str_url=req->url().toString();

            // qInfo() << "url:" << str_url;

            if(str_url.contains("favicon", Qt::CaseInsensitive)) {
                res->end(favicon());
                return;
            }

            if(str_url.contains(QStringLiteral("index.css"), Qt::CaseInsensitive)) {
                res->end(cssIndex());
                return;
            }


            if(str_url==QStringLiteral("/") || str_url.contains(QStringLiteral("index"), Qt::CaseInsensitive)) {
                res->end(pageIndex());
                return;
            }


            if(str_url.contains(QStringLiteral("settings"), Qt::CaseInsensitive)) {
                if(!url.queryItems().isEmpty())
                    checkSettings(queryListToMap(url.queryItems()));

                res->end(pageSettings());
                return;
            }


            if(QDateTime::currentMSecsSinceEpoch() - last_buf_update>=100) {
                ba_buffer=QJsonDocument::fromVariant(status.toExt()).toJson(QJsonDocument::Compact);

                last_buf_update=QDateTime::currentMSecsSinceEpoch();
            }

            res->end(ba_buffer);
        }
    });

#endif
}

HttpServer::~HttpServer()
{
}

void HttpServer::setSettingsModel(SettingsModel *mdl)
{
    settings_model=mdl;
}

void HttpServer::formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format)
{
    status.input_format={ width, height, frame_duration, frame_scale, progressive_frame, pixel_format };
}

void HttpServer::setRecState(const bool &value)
{
    if(!value) {
        status.rec_stats=NRecStats();
    }
}

void HttpServer::setRecStats(const NRecStats &value)
{
    status.rec_stats=value;
}

void HttpServer::setPlayerDuration(const qint64 &value)
{
    status.player_state.duration=value;
}

void HttpServer::setPlayerPosition(const qint64 &value)
{
    status.player_state.position=value;
}

void HttpServer::setFreeSpace(const qint64 &value)
{
    status.free_space=value;
}

QByteArray HttpServer::pageIndex()
{
    QString page;

    page+=QStringLiteral("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"><title>capturer::state</title>"
                         "<link rel='stylesheet' type='text/css' href='index.css'>"
                         "</head>");
    page+=QStringLiteral("<script>");

    page+=QStringLiteral("function onLoad() {\n"
                         "  if(window.location.href.split(\"?\").length>1) {\n"
                         "    window.location.replace(location.pathname);\n"
                         "  }\n"
                         "  loadData();\n"
                         "  setInterval('loadData()', 1000);\n"
                         "}\n");

    page+=QStringLiteral("function loadData() {\n"
                         "  var request=new XMLHttpRequest();\n"
                         "  request.open('GET', 'http://' + window.location.host + '/data');\n"

                         "  request.onreadystatechange=function () {\n"
                         "    if(request.status!=200) {\n"
                         "      document.getElementById('input_format').innerHTML=' offline';\n"
                         "      document.getElementById('rec_stats').style.visibility='hidden';\n"
                         "      document.getElementById('button_rec').innerHTML='start rec';\n"
                         "    }\n"
                         "  }\n"

                         "  request.onload=function() {\n"

                         "    var data=JSON.parse(request.responseText);\n"

                         "    if(data.input_format.pixel_format.length===0) {\n"
                         "      document.getElementById('input_format').innerHTML='';\n"

                         "    } else {"
                         "      var str=data.input_format.height + (data.input_format.progressive_frame ? 'p' : 'i') + '@'"
                         "       + (data.input_format.frame_scale/data.input_format.frame_duration).toFixed(2) + ' ' + data.input_format.pixel_format;\n"

                         "      document.getElementById('input_format').innerHTML=' ' + str;\n"
                         "    }\n"

                         "    if(data.rec_stats.time==null) {\n"
                         "      document.getElementById('rec_stats').style.visibility='hidden';\n"
                         "      document.getElementById('button_rec').innerHTML='start rec';\n"

                         "    } else {\n"
                         "      document.getElementById('rec_stats').style.visibility='visible';\n"

                         "      document.getElementById('button_rec').innerHTML='stop rec';\n"

                         "      var rec_stats=data.rec_stats;\n"

                         "      document.getElementById('time').innerHTML=rec_stats.time.substring(0, 8);\n"
                         "      document.getElementById('free_space').innerHTML=(data.free_space/1024/1024).toFixed(0) + ' MB';\n"
                         "      document.getElementById('size').innerHTML=(rec_stats.size/1024/1024).toFixed(0) + ' MB';\n"
                         "      document.getElementById('avg_bitrate').innerHTML=(rec_stats.avg_bitrate/8/1024/1024).toFixed(2) + ' MB/s';\n"
                         "      document.getElementById('frame_buffer').innerHTML=rec_stats.frame_buffer_used + '/' + rec_stats.frame_buffer_size;\n"
                         "      document.getElementById('frames_dropped').innerHTML=rec_stats.dropped_frames_counter;\n"
                         "    }\n"
                         "  };\n"

                         "  request.send();\n"
                         "}\n"
                         );

    page+=QStringLiteral("function sendCmd(key_code) {\n"
                         "  var request=new XMLHttpRequest();\n"
                         "  request.open('GET', 'http://' + window.location.host + '/index?key_code=' + key_code);\n"
                         "  request.send();\n"
                         "}\n");

    page+=QStringLiteral("</script>");

    page+=QStringLiteral("<body class='root' onload='onLoad();'>");


    page+=QStringLiteral("<div class='row_1'><form><button style='width:128' type='reset' onclick=\"location.href='/settings'\">settings</button></form></div>");

    page+=QString("<button class='button_rec' id='button_rec' onclick='sendCmd(%1);'></button>").arg(KeyCodeC::Rec);


    page+="<div class='row_3'>";

    page+=QString("<span>input format:</span><span id='input_format'></span>");

    TableBuilder table_builder;

    // page+=table_builder.open(2, "display:none", "", "id='rec_stats' ");
    page+=table_builder.open(2, "visibility:hidden", "", "id='rec_stats' ", 0, 0, 4);


    page+=table_builder.row(QStringList() << QStringLiteral("time:") << "<span id='time'></span>");
    page+=table_builder.row(QStringList() << QStringLiteral("free space:") << "<span id='free_space'></span>");
    page+=table_builder.row(QStringList() << QStringLiteral("size:") << "<span id='size'></span>");

    page+=table_builder.row(QStringList() << QStringLiteral("avg bitrate:") << "<span id='avg_bitrate'></span>");

    page+=table_builder.row(QStringList() << QStringLiteral("frame buffer:") << "<span id='frame_buffer'></span>");
    page+=table_builder.row(QStringList() << QStringLiteral("frames dropped:") << "<span id='frames_dropped'></span>");
    page+=table_builder.close();

    page+="</div>";
    page+=QStringLiteral("</body>");
    page+=QStringLiteral("</html>");

    return page.toUtf8();
}

QString tagBold(const QString &str)
{
    return QString("<b>%1</b>").arg(str);
}

QString tagCenter(const QString &str)
{
    return QString("<center>%1</center>").arg(str);
}

QString nameToMarker(QString name)
{
    static QHash <QString, QString> mark;

    if(!mark.contains(name)) {
        bool is_number;

        name.toInt(&is_number, 10);

        if(is_number)
            mark[name]=name;

        else
            mark[name]=QLatin1String("v_") + name
                    .replace(QLatin1Char(' '), QLatin1Char('_'))
                    .replace(QLatin1Char('/'), QLatin1Char('_'))
                    .replace(QLatin1Char('.'), QLatin1Char('_'))
                    .replace(QLatin1Char(','), QLatin1Char('_'))
                    .replace(QLatin1Char('-'), QLatin1Char('_'))
                    .replace(QLatin1Char('('), QLatin1Char('_'))
                    .replace(QLatin1Char(')'), QLatin1Char('_'))
                    .toLower();
    }

    return mark[name];
}

QString addCheckbox(const QString &key, bool checked)
{
    return QString("<input type='checkbox'%2 onchange=\"if(this.checked) document.getElementById('%1').value='1'; else document.getElementById('%1').value='0'\"/><input type='hidden' name='%1' id='%1' value='%3'/>")
            .arg(nameToMarker(key))
            .arg(checked ? QStringLiteral(" checked") : "")
            .arg(checked ? QStringLiteral("1") : QStringLiteral("0"));
}

QString addCombobox(const QString &key, int value, QStringList values)
{
    QString str=QString("<select name='%1' style='width:100%'>").arg(nameToMarker(key));

    for(int i=0; i<values.size(); ++i) {
        QString val=values[i];

        if(value==i)
            str+=QString("<option value='%1' selected>%2</option>").arg(i).arg(val);

        else
            str+=QString("<option value='%1'>%2</option>").arg(i).arg(val);
    }

    str+=QStringLiteral("</select>");

    return str;
}

QString addButton(const QString &key, const QString &name)
{
    return QString("<button type='reset' style='width:100%' onclick=\"location.href='/settings?%1'\">%2</button>")
            .arg(nameToMarker(key))
            .arg(name);
}

QByteArray HttpServer::pageSettings()
{
    if(!settings_model)
        return QByteArray();

    QString page;

    page+=QStringLiteral("<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"><title>capturer::settings</title></head>");

    page+=QStringLiteral("<script>function checkArgs() { if(window.location.href.split(\"?\").length>1) { window.location.href='/settings'; } }</script>");

    page+=QStringLiteral("<body onload='checkArgs();' style='background:linear-gradient(200deg, #c05c7c, #d56a59)'>");

    TableBuilder table_builder;


    page+=QStringLiteral("<form><br><center><button style='width:128' type='reset' onclick=\"location.href='/'\">home</button></center><br></form>");

    page+=QStringLiteral("<form method='GET' action=''>");


    if(status.rec_stats.isNull())
        page+=QStringLiteral("<fieldset style='border: 0'>");

    else
        page+=QStringLiteral("<fieldset style='border: 0' disabled>");


    page+=QStringLiteral("<center>");


    page+=QStringLiteral("<button style='width:128' type='submit'>apply settings</button><br><br>");


    page+=table_builder.open(2, "", "", "", 0, 2, 2);

    for(int row=0; row<settings_model->rowCount(); ++row) {
        if(settings_model->data(row, SettingsModel::Role::type).toInt()==SettingsModel::Type::title) {
            page+=table_builder.row(QStringList()
                                    << tagCenter(tagBold(settings_model->data(row, SettingsModel::Role::name).toString()))
                                    , QStringList() << QStringLiteral("colspan=2"));

        } else if(settings_model->data(row, SettingsModel::Role::type).toInt()==SettingsModel::Type::divider) {
            page+=table_builder.row(QStringList() << QStringLiteral("<br>"), QStringList() << QStringLiteral("colspan=2"));

        } else if(settings_model->data(row, SettingsModel::Role::type).toInt()==SettingsModel::Type::combobox) {
            page+=table_builder.row(QStringList()
                                    << settings_model->data(row, SettingsModel::Role::name).toString() + ":"
                                    << addCombobox(settings_model->data(row, SettingsModel::Role::group).toString() + "_"
                                                   + settings_model->data(row, SettingsModel::Role::name).toString()
                                                   , settings_model->data(row, SettingsModel::Role::value).toInt()
                                                   , settings_model->data(row, SettingsModel::Role::values).toStringList()));

        } else if(settings_model->data(row, SettingsModel::Role::type).toInt()==SettingsModel::Type::checkbox) {
            page+=table_builder.row(QStringList() << settings_model->data(row, SettingsModel::Role::name).toString() + ":" <<
                                    tagCenter(addCheckbox(settings_model->data(row, SettingsModel::Role::group).toString() + "_"
                                                          + settings_model->data(row, SettingsModel::Role::name).toString()
                                                          , settings_model->data(row, SettingsModel::Role::value).toBool())));

        } else if(settings_model->data(row, SettingsModel::Role::type).toInt()==SettingsModel::Type::button) {
            page+=table_builder.row(QStringList()
                                    << ""
                                    << addButton(settings_model->data(row, SettingsModel::Role::group).toString() + "_"
                                                 + settings_model->data(row, SettingsModel::Role::name).toString(),
                                                 settings_model->data(row, SettingsModel::Role::name).toString()));

        } else {
            qWarning() << "unhandled type:" << settings_model->data(row, SettingsModel::Role::type).toInt();
        }
    }

    page+=table_builder.close();

    page+=QStringLiteral("</center>");
    page+=QStringLiteral("</fieldset>");
    page+=QStringLiteral("</form>");
    page+=QStringLiteral("</body>");
    page+=QStringLiteral("</html>");

    return page.toUtf8();
}

QByteArray HttpServer::favicon()
{
    static QByteArray d;

    if(d.isEmpty()) {
        QPixmap pm=QIcon(QStringLiteral(":/images/capturer.svg")).pixmap(QSize(128, 128));
        QBuffer buffer;
        pm.save(&buffer, "PNG");
        d=buffer.buffer();
    }

    return d;
}

QByteArray HttpServer::cssIndex()
{
    static QByteArray d;

    if(d.isEmpty()) {
        d+=
                ".root {"
                "  background: linear-gradient(200deg, #c05c7c, #d56a59);"
                "  display: grid;"
                "  grid-template-columns: repeat(3, 1fr);"
                "  grid-template-rows: repeat(3, 1fr);"
                "  color: white;"
                "  text-shadow: black 0.1em 0.1em 0.2em;"
                "}"

                ".button_rec {"
                "  grid-column: 2;"
                "  grid-row: 2;"
                "  align-self: center;"
                "  justify-self: center;"
                "  // cursor: none;"
                "  outline: none;"
                "  background: radial-gradient(#aa0000, #bb0000, #ff0000);"
                "  border-radius: 50%;"
                "  border-width: 14px;"
                "  border-color: #ff4242 #ff0000 #dd0000;"
                "  padding: 64px;"
                "  color: white;"
                "  font-size: 20px;"
                "  height: 240px;"
                "  width: 240px;"
                "}"

                ".button_rec:active {"
                "  border-width: 4px;"
                "  font-size: 18px;"
                "}"

                ".row_1 {"
                "  grid-column: 2;"
                "  grid-row: 1;"
                "  align-self: center;"
                "  justify-self: center;"
                "}"

                ".row_3 {"
                "  grid-column: 2;"
                "  grid-row: 3;"
                "  align-self: center;"
                "  justify-self: center;"
                "}"
                ;
    }

    return d;
}

void HttpServer::checkSettings(QMap <QString, QString> new_settings)
{
    if(!status.rec_stats.isNull())
        return;

    QString key;
    int value;

    QStringList skip_group;

    bool data_changed=false;


    for(int row=0; row<settings_model->rowCount(); ++row) {
        if(skip_group.contains(settings_model->data(row, SettingsModel::Role::group).toString()))
            continue;

        key=nameToMarker(settings_model->data(row, SettingsModel::Role::group).toString() + "_" + settings_model->data(row, SettingsModel::Role::name).toString());

        if(new_settings.contains(key)) {
            if(settings_model->data_p(row)->value==&settings->device_cam.restart) {
                qInfo() << "device_cam.restart";
                emit deviceCamRestart();
                return;

            } else if(settings_model->data_p(row)->value==&settings->device_cam.stop) {
                qInfo() << "device_cam.stop";
                emit deviceCamStop();
                return;

            } else if(settings_model->data_p(row)->value==&settings->device_decklink.restart) {
                qInfo() << "device_decklink.restart";
                emit deviceDecklinkRestart();
                return;

            } else if(settings_model->data_p(row)->value==&settings->rec.check_encoders) {
                qInfo() << "rec.check_encoders";
                emit checkEncoders();
                return;
            }

            value=new_settings.value(key).toInt();

            if(value!=settings_model->data(row, SettingsModel::Role::value).toInt()) {
                if(settings_model->data(row, SettingsModel::Role::priority).toInt()==SettingsModel::Priority::high)
                    skip_group.append(settings_model->data(row, SettingsModel::Role::group).toString());

                settings_model->setData(row, SettingsModel::Role::value, value);

                data_changed=true;
            }
        }
    }

    if(data_changed)
        settings_model->updateQml();
}

