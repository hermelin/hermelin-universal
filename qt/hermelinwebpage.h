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

#ifndef HERMELINWEBPAGE_H
#define HERMELINWEBPAGE_H

#include "common.h"

// Qt
#include <QWebPage>
#include <QByteArray>

class HermelinRequest;
class MainWindow;
class HermelinWebPage : public QWebPage
{
    Q_OBJECT
public:
    HermelinWebPage(MainWindow *mainWindow, QObject* parent = 0);
protected Q_SLOTS:
    void requestFinished(HermelinRequest* request, QByteArray result, QString uuid , bool error);
protected:
    virtual bool acceptNavigationRequest(QWebFrame * frame, const QNetworkRequest & request, NavigationType type);
    virtual void javaScriptAlert(QWebFrame * frame, const QString & msg);
#ifdef MEEGO_EDITION_HARMATTAN
    virtual bool javaScriptConfirm(QWebFrame *frame, const QString &msg);
#endif
    bool handleUri(const QString& string);
private:
    MainWindow* m_mainWindow;
};

#endif // HermelinWebPage_H
