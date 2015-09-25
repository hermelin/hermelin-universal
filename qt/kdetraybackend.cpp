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
#include <QPainter>
#include <QDebug>
#include <QApplication>

// KDE
#include <KStatusNotifierItem>
#include <KMenu>
#include <KIcon>
#include <KGlobalSettings>
#include <KColorScheme>
#include <KGlobal>
#include <KComponentData>
#include <KAboutData>

// Hermelin
#include "mainwindow.h"
#include "kdetraybackend.h"

KDETrayBackend::KDETrayBackend(MainWindow* parent):
    QObject(parent),
    TrayIconInterface(),
    m_mainWindow(parent),
    m_statusNotifierItem(new KStatusNotifierItem("hermelin_qt", this))
{
    m_statusNotifierItem->setAssociatedWidget(parent);
    m_statusNotifierItem->setIconByName("hermelin_qt-inactive");
    m_statusNotifierItem->setToolTip("hermelin", i18n("Hermelin"), "");
    m_statusNotifierItem->setStatus(KStatusNotifierItem::Active);
    m_statusNotifierItem->setCategory(KStatusNotifierItem::Communications);
    m_statusNotifierItem->setStandardActionsEnabled(false);
    m_statusNotifierItem->setToolTipTitle(i18n("Hermelin"));

    connect(m_statusNotifierItem, SIGNAL(activateRequested(bool, QPoint)), this, SLOT(activate(bool, QPoint)));
}

void KDETrayBackend::showMessage(QString type, QString title, QString message, QString image)
{
    Q_UNUSED(type)
    Q_UNUSED(image)
    m_statusNotifierItem->showMessage(title, message, "hermelin", 4000);
}

void KDETrayBackend::setContextMenu(QMenu* menu)
{
    KMenu* kmenu = m_statusNotifierItem->contextMenu();
    Q_FOREACH(QAction * action, kmenu->actions()) {
        kmenu->removeAction(action);
    }
    kmenu->addTitle(qApp->windowIcon(), KGlobal::caption());
    kmenu->setTitle(KGlobal::mainComponent().aboutData()->programName());
    Q_FOREACH(QAction * action, menu->actions()) {
        kmenu->addAction(action);
    }
}

void KDETrayBackend::activate(bool active, const QPoint& pos)
{
//    m_mainWindow->triggerVisible();
}

void KDETrayBackend::unreadAlert(QString number)
{
    int n = number.toInt();
    if (n > 0) {
        m_statusNotifierItem->setIconByName("hermelin_qt-active");
    } else
        m_statusNotifierItem->setIconByName("hermelin_qt-inactive");
    m_statusNotifierItem->setToolTip("hermelin", i18n("Hermelin"), i18np("1 unread post", "%1 unread posts", n));
}
