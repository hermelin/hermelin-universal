/***************************************************************************
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "common.h"

// Qt
#include <QDebug>
#include <QProcess>
#include <QClipboard>
#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QFileDialog>
#include <QWebFrame>

#ifdef MEEGO_EDITION_HARMATTAN
#include <MMessageBox>
#endif

// Hermelin
#include "hermelinwebpage.h"
#include "hermelinrequest.h"
#include "mainwindow.h"

HermelinWebPage::HermelinWebPage(MainWindow *window, QObject* parent) :
    QWebPage(parent)
{
    m_mainWindow = window;
    networkAccessManager()->setProxy(QNetworkProxy::DefaultProxy);
}

bool HermelinWebPage::acceptNavigationRequest(QWebFrame * frame, const QNetworkRequest & request, NavigationType type)
{
    Q_UNUSED(frame);
    Q_UNUSED(type);
    return handleUri(request.url().toString());
}

bool HermelinWebPage::handleUri(const QString& originmsg)
{
    QString msg = originmsg;
    if (msg.startsWith("hermelin:")) {
        msg = msg.mid(6);
        QString type = msg.section("/", 0, 0);
        QString method = msg.section("/", 1, 1);
        if (type == "system") {
            if (method == "notify") {
                QString notify_type = QUrl::fromPercentEncoding(msg.section("/", 2, 2).toUtf8());
                QString title = QUrl::fromPercentEncoding(msg.section("/", 3, 3).toUtf8());
                QString summary = QUrl::fromPercentEncoding(msg.section("/", 4, 4).toUtf8());
                QString image = QUrl::fromPercentEncoding(msg.section("/", 5, 5).toUtf8());

                m_mainWindow->notification(notify_type, title, summary, image);
            } else if (method == "unread_alert") {
                QString number = QUrl::fromPercentEncoding(msg.section("/", 2, 2).toUtf8());
                m_mainWindow->unreadAlert(number);
            } else if (method == "load_settings") {
                QString settingString = QUrl::fromPercentEncoding(msg.section("/", 2, -1).toUtf8());
                currentFrame()->evaluateJavaScript("hermelin_qt = " + settingString + ";");
                QString proxyType = currentFrame()->evaluateJavaScript("hermelin_qt.proxy_type").toString();
                QNetworkProxy proxy;
                QNetworkAccessManager* nm = NULL;
#ifdef HAVE_KDE
                if (proxyType == "none") {
                    nm = new QNetworkAccessManager(this);
                }
#else
                if (proxyType == "system")
                {
                    nm = new QNetworkAccessManager(this);
                    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery();
                    proxy = proxies[0];
                }
#endif
                if (proxyType == "http" || proxyType == "socks") {
                    nm = new QNetworkAccessManager(this);
                    bool proxyAuth = currentFrame()->evaluateJavaScript("hermelin_qt.proxy_auth").toBool();
                    int proxyPort = currentFrame()->evaluateJavaScript("hermelin_qt.proxy_port").toInt();
                    QString proxyHost = currentFrame()->evaluateJavaScript("hermelin_qt.proxy_host").toString();
                    QString proxyAuthName = currentFrame()->evaluateJavaScript("hermelin_qt.proxy_auth_name").toString();
                    QString proxyAuthPassword = currentFrame()->evaluateJavaScript("hermelin_qt.proxy_auth_password").toString();

                    proxy = QNetworkProxy(proxyType == "socks" ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy,
                                        proxyHost,
                                        proxyPort);

                    if (proxyAuth) {
                        proxy.setUser(proxyAuthName);
                        proxy.setPassword(proxyAuthPassword);
                    }
                }
                if (proxy.type() != QNetworkProxy::NoProxy) {
                    QNetworkProxy::setApplicationProxy(proxy);
                }

                if (nm != NULL) {
                    QNetworkAccessManager* oldnm = networkAccessManager();
                    oldnm->setParent(NULL);
                    oldnm->deleteLater();
                    nm->setProxy(QNetworkProxy::DefaultProxy);
                    setNetworkAccessManager(nm);
                }
            } else if (method == "sign_in") {
                m_mainWindow->setSignIn(true);
            } else if (method == "sign_out") {
                m_mainWindow->setSignIn(false);
            }
        } else if (type == "action") {
            if (method == "search") {

            } else if (method == "choose_file") {
                QFileDialog dialog;
                dialog.setAcceptMode(QFileDialog::AcceptOpen);
                dialog.setFileMode(QFileDialog::ExistingFile);
                dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg *.gif)"));
                int result = dialog.exec();
                if (result) {
                    QStringList fileNames = dialog.selectedFiles();
                    if (fileNames.size() > 0) {
                        QString callback = msg.section("/", 2, 2);
                        currentFrame()->evaluateJavaScript(QString("%1(\"%2\")").arg(callback, QUrl::fromLocalFile(fileNames[0]).toString().replace("file://", "")));
                    }
                }
            } else if (method == "save_avatar") {
            } else if (method == "log") {
                qDebug() << msg;
            } else if (method == "paste_clipboard_text") {
                triggerAction(QWebPage::Paste);
            } else if (method == "set_clipboard_text") {
                QClipboard *clipboard = QApplication::clipboard();
                if (clipboard)
                    clipboard->setText(msg.section("/", 2, -1));
            }
        } else if (type == "request") {
            QString json = QUrl::fromPercentEncoding(msg.section("/", 1, -1).toUtf8());
            currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json = %1 ;").arg(json));
            QString request_uuid = currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json.uuid")).toString();
            QString request_method = currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json.method")).toString();
            QString request_url = currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json.url")).toString();
            QMap<QString, QVariant> request_params = currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json.params")).toMap();
            QMap<QString, QVariant> request_headers = currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json.headers")).toMap();
            QList<QVariant> request_files = currentFrame()->evaluateJavaScript(QString("hermelin_qt_request_json.files")).toList();

            HermelinRequest* request = new HermelinRequest(
                request_uuid,
                request_method,
                request_url,
                request_params,
                request_headers,
                request_files,
                userAgentForUrl(request_url),
                networkAccessManager());
            connect(request, SIGNAL(requestFinished(HermelinRequest*, QByteArray, QString, bool)), this, SLOT(requestFinished(HermelinRequest*, QByteArray, QString, bool)));
            if (!request->doRequest())
                delete request;
        }
    } else if (msg.startsWith("file://") || msg.startsWith("qrc:")) {
        return true;
    } else if (msg.startsWith("about:")) {
        return false;
    } else if (msg.startsWith("http://stat.hotot.org")) {
        return false;
    } else {
        QDesktopServices::openUrl(msg);
    }
    return false;
}

void HermelinWebPage::javaScriptAlert(QWebFrame *frame, const QString &msg)
{
    Q_UNUSED(frame);
    handleUri(msg);
}

#ifdef MEEGO_EDITION_HARMATTAN
bool HermelinWebPage::javaScriptConfirm(QWebFrame *frame, const QString &msg)
{
    Q_UNUSED(frame);
    MMessageBox *messageBox = new MMessageBox(msg, M::YesButton|M::NoButton);
    //int result = messageBox->exec(m_mainWindow);
    messageBox->appear(MSceneWindow::DestroyWhenDone);
    int result = 0;
    return (result == MDialog::Accepted);
}

#endif

void HermelinWebPage::requestFinished(HermelinRequest* request, QByteArray result, QString uuid , bool error)
{
    QString strresult = QString::fromUtf8(result);
    if (error) {
        QString scripts = QString("widget.DialogManager.alert('%1', '%2');\n"
                                  "globals.network.error_task_table['%3']('');\n"
                                 ).arg("Ooops, an Error occurred!", strresult, uuid);
        currentFrame()->evaluateJavaScript(scripts);
    } else {
        QString scripts;
        if (strresult.startsWith("[") || strresult.startsWith("{"))
            scripts = QString("globals.network.success_task_table['%1'](%2);"
                             ).arg(uuid, strresult);
        else
            scripts = QString("globals.network.success_task_table['%1']('%2');"
                             ).arg(uuid, strresult);
        currentFrame()->evaluateJavaScript(scripts);
    }
    request->deleteLater();
}
