/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdesigner_membersheet_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <abstractintrospection_p.h>

#include <QtWidgets/QWidget>

namespace {

class Qt3Members
  {
  public:
      static Qt3Members *instance();
      QMap<QString, QStringList> getSignals() const { return m_classNameToSignals; }
      QMap<QString, QStringList> getSlots() const { return m_classNameToSlots; }
  private:
      Qt3Members();
      static Qt3Members *m_instance;
      QMap<QString, QStringList> m_classNameToSignals;
      QMap<QString, QStringList> m_classNameToSlots;
  };

Qt3Members *Qt3Members::m_instance = 0;

Qt3Members::Qt3Members()
{
    m_classNameToSignals[QStringLiteral("QTextEdit")].append(QStringLiteral("currentFontChanged(QFont)"));
    m_classNameToSignals[QStringLiteral("QTextEdit")].append(QStringLiteral("currentColorChanged(QColor)"));
    m_classNameToSignals[QStringLiteral("QTabWidget")].append(QStringLiteral("currentChanged(QWidget*)"));
    m_classNameToSignals[QStringLiteral("QTabWidget")].append(QStringLiteral("selected(QString)"));
    m_classNameToSignals[QStringLiteral("QTabBar")].append(QStringLiteral("selected(int)"));
    m_classNameToSignals[QStringLiteral("QMenuBar")].append(QStringLiteral("activated(int)"));
    m_classNameToSignals[QStringLiteral("QMenuBar")].append(QStringLiteral("highlighted(int)"));
    m_classNameToSignals[QStringLiteral("QMenu")].append(QStringLiteral("activated(int)"));
    m_classNameToSignals[QStringLiteral("QMenu")].append(QStringLiteral("highlighted(int)"));
    m_classNameToSignals[QStringLiteral("QLineEdit")].append(QStringLiteral("lostFocus()"));
    m_classNameToSignals[QStringLiteral("QDial")].append(QStringLiteral("dialPressed()"));
    m_classNameToSignals[QStringLiteral("QDial")].append(QStringLiteral("dialMoved(int)"));
    m_classNameToSignals[QStringLiteral("QDial")].append(QStringLiteral("dialReleased()"));
    m_classNameToSignals[QStringLiteral("QComboBox")].append(QStringLiteral("textChanged(QString)"));
    m_classNameToSignals[QStringLiteral("QActionGroup")].append(QStringLiteral("selected(QAction*)"));
    m_classNameToSignals[QStringLiteral("QAction")].append(QStringLiteral("activated(int)"));
    m_classNameToSignals[QStringLiteral("QAbstractSocket")].append(QStringLiteral("connectionClosed()"));
    m_classNameToSignals[QStringLiteral("QAbstractSocket")].append(QStringLiteral("delayedCloseFinished()"));

    m_classNameToSlots[QStringLiteral("QWidget")].append(QStringLiteral("setShown(bool)"));
    m_classNameToSlots[QStringLiteral("QToolButton")].append(QStringLiteral("setTextPosition(QToolButton::TextPosition)"));
    m_classNameToSlots[QStringLiteral("QToolButton")].append(QStringLiteral("setUsesBigPixmap(bool)"));
    m_classNameToSlots[QStringLiteral("QToolButton")].append(QStringLiteral("setUsesTextLabel(bool)"));
    m_classNameToSlots[QStringLiteral("QTextEdit")].append(QStringLiteral("setModified(bool)"));
    m_classNameToSlots[QStringLiteral("QTextEdit")].append(QStringLiteral("setColor(QColor)"));
    m_classNameToSlots[QStringLiteral("QTabWidget")].append(QStringLiteral("setCurrentPage(int)"));
    m_classNameToSlots[QStringLiteral("QTabWidget")].append(QStringLiteral("showPage(QWidget*)"));
    m_classNameToSlots[QStringLiteral("QTabWidget")].append(QStringLiteral("removePage(QWidget*)"));
    m_classNameToSlots[QStringLiteral("QTabBar")].append(QStringLiteral("setCurrentTab(int)"));
    m_classNameToSlots[QStringLiteral("QStatusBar")].append(QStringLiteral("message(QString,int)"));
    m_classNameToSlots[QStringLiteral("QStatusBar")].append(QStringLiteral("clear()"));
    m_classNameToSlots[QStringLiteral("QSplashScreen")].append(QStringLiteral("message(QString,int)"));
    m_classNameToSlots[QStringLiteral("QSplashScreen")].append(QStringLiteral("clear()"));
    m_classNameToSlots[QStringLiteral("QSlider")].append(QStringLiteral("addStep()"));
    m_classNameToSlots[QStringLiteral("QSlider")].append(QStringLiteral("subtractStep()"));
    m_classNameToSlots[QStringLiteral("QAbstractButton")].append(QStringLiteral("setOn(bool)"));
    m_classNameToSlots[QStringLiteral("QAction")].append(QStringLiteral("setOn(bool)"));
    m_classNameToSlots[QStringLiteral("QErrorMessage")].append(QStringLiteral("message(QString)"));
    m_classNameToSlots[QStringLiteral("QTimer")].append(QStringLiteral("changeInterval(int)"));
    m_classNameToSlots[QStringLiteral("QTimer")].append(QStringLiteral("start(int,bool)"));
}

Qt3Members *Qt3Members::instance()
{
    if (!m_instance)
        m_instance = new Qt3Members();
    return m_instance;
}
}

QT_BEGIN_NAMESPACE

static QList<QByteArray> stringListToByteArray(const QStringList &l)
{
    if (l.empty())
        return QList<QByteArray>();
    QList<QByteArray> rc;
    const QStringList::const_iterator cend = l.constEnd();
    for (QStringList::const_iterator it = l.constBegin(); it != cend; ++it)
        rc += it->toUtf8();
    return rc;
}

// Find the form editor in the hierarchy.
// We know that the parent of the sheet is the extension manager
// whose parent is the core.

static QDesignerFormEditorInterface *formEditorForObject(QObject *o) {
    do {
        if (QDesignerFormEditorInterface* core = qobject_cast<QDesignerFormEditorInterface*>(o))
            return core;
        o = o->parent();
    } while(o);
    Q_ASSERT(o);
    return 0;
}

// ------------ QDesignerMemberSheetPrivate
class QDesignerMemberSheetPrivate {
public:
    explicit QDesignerMemberSheetPrivate(QObject *object, QObject *sheetParent);

    QDesignerFormEditorInterface *m_core;
    const QDesignerMetaObjectInterface *m_meta;

    class Info {
    public:
        inline Info() : visible(true) {}

        QString group;
        bool visible;
    };

    typedef QHash<int, Info> InfoHash;

    Info &ensureInfo(int index);

    InfoHash m_info;
};

QDesignerMemberSheetPrivate::QDesignerMemberSheetPrivate(QObject *object, QObject *sheetParent) :
    m_core(formEditorForObject(sheetParent)),
    m_meta(m_core->introspection()->metaObject(object))
{
}

QDesignerMemberSheetPrivate::Info &QDesignerMemberSheetPrivate::ensureInfo(int index)
{
    InfoHash::iterator it = m_info.find(index);
    if (it == m_info.end()) {
        it = m_info.insert(index, Info());
    }
    return it.value();
}

// --------- QDesignerMemberSheet

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent) :
    QObject(parent),
    d(new QDesignerMemberSheetPrivate(object, parent))
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
    delete d;
}

int QDesignerMemberSheet::count() const
{
    return d->m_meta->methodCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
    return d->m_meta->indexOfMethod(name);
}

QString QDesignerMemberSheet::memberName(int index) const
{
    return d->m_meta->method(index)->tag();
}

QString QDesignerMemberSheet::declaredInClass(int index) const
{
    const QString member = d->m_meta->method(index)->signature();

    // Find class whose superclass does not contain the method.
    const QDesignerMetaObjectInterface *meta_obj = d->m_meta;

    for (;;) {
        const QDesignerMetaObjectInterface *tmp = meta_obj->superClass();
        if (tmp == 0)
            break;
        if (tmp->indexOfMethod(member) == -1)
            break;
        meta_obj = tmp;
    }
    return meta_obj->className();
}

QString QDesignerMemberSheet::memberGroup(int index) const
{
    return d->m_info.value(index).group;
}

void QDesignerMemberSheet::setMemberGroup(int index, const QString &group)
{
    d->ensureInfo(index).group = group;
}

QString QDesignerMemberSheet::signature(int index) const
{
    return d->m_meta->method(index)->normalizedSignature();
}

bool QDesignerMemberSheet::isVisible(int index) const
{
    typedef QDesignerMemberSheetPrivate::InfoHash InfoHash;
    const InfoHash::const_iterator it = d->m_info.constFind(index);
    if (it != d->m_info.constEnd())
        return it.value().visible;

   return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Signal
           || d->m_meta->method(index)->access() == QDesignerMetaMethodInterface::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
    d->ensureInfo(index).visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
    return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
    return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Slot;
}

bool QDesignerMemberSheet::inheritedFromWidget(int index) const
{
    const QString name = d->m_meta->method(index)->signature();
    return declaredInClass(index) == QStringLiteral("QWidget") || declaredInClass(index) == QStringLiteral("QObject");
}


QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{
    return stringListToByteArray(d->m_meta->method(index)->parameterTypes());
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
    return stringListToByteArray(d->m_meta->method(index)->parameterNames());
}

bool QDesignerMemberSheet::signalMatchesSlot(const QString &signal, const QString &slot)
{
    bool result = true;

    do {
        int signal_idx = signal.indexOf(QLatin1Char('('));
        int slot_idx = slot.indexOf(QLatin1Char('('));
        if (signal_idx == -1 || slot_idx == -1)
            break;

        ++signal_idx; ++slot_idx;

        if (slot.at(slot_idx) == QLatin1Char(')'))
            break;

        while (signal_idx < signal.size() && slot_idx < slot.size()) {
            const QChar signal_c = signal.at(signal_idx);
            const QChar slot_c = slot.at(slot_idx);

            if (signal_c == QLatin1Char(',') && slot_c == QLatin1Char(')'))
                break;

            if (signal_c == QLatin1Char(')') && slot_c == QLatin1Char(')'))
                break;

            if (signal_c != slot_c) {
                result = false;
                break;
            }

            ++signal_idx; ++slot_idx;
        }
    } while (false);

    return result;
}

bool QDesignerMemberSheet::isQt3Signal(int index) const
{
    if (!isSignal(index))
        return false;

    const QString className = declaredInClass(index);
    const QString signalSignature = signature(index);

    QMap<QString, QStringList> qt3signals = Qt3Members::instance()->getSignals();
    QMap<QString, QStringList>::const_iterator it = qt3signals.constFind(className);
    if (it != qt3signals.constEnd() && (*it).contains(signalSignature))
        return true;

    return false;
}

bool QDesignerMemberSheet::isQt3Slot(int index) const
{
    if (!isSlot(index))
        return false;

    const QString className = declaredInClass(index);
    const QString slotSignature = signature(index);

    QMap<QString, QStringList> qt3slots = Qt3Members::instance()->getSlots();
    QMap<QString, QStringList>::const_iterator it = qt3slots.constFind(className);
    if (it != qt3slots.constEnd() && (*it).contains(slotSignature))
        return true;
    return false;
}

// ------------ QDesignerMemberSheetFactory

QDesignerMemberSheetFactory::QDesignerMemberSheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerMemberSheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerMemberSheetExtension)) {
        return new QDesignerMemberSheet(object, parent);
    }

    return 0;
}

QT_END_NAMESPACE
