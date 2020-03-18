/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt UI Tools library of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QUILOADER_H
#define QUILOADER_H

#include <QtUiTools/qtuitoolsglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QWidget;
class QLayout;
class QAction;
class QActionGroup;
class QString;
class QIODevice;
class QDir;

class QUiLoaderPrivate;
class Q_UITOOLS_EXPORT QUiLoader : public QObject
{
    Q_OBJECT
public:
    explicit QUiLoader(QObject *parent = nullptr);
    ~QUiLoader() override;

    QStringList pluginPaths() const;
    void clearPluginPaths();
    void addPluginPath(const QString &path);

    QWidget *load(QIODevice *device, QWidget *parentWidget = nullptr);
    QStringList availableWidgets() const;
    QStringList availableLayouts() const;

    virtual QWidget *createWidget(const QString &className, QWidget *parent = nullptr, const QString &name = QString());
    virtual QLayout *createLayout(const QString &className, QObject *parent = nullptr, const QString &name = QString());
    virtual QActionGroup *createActionGroup(QObject *parent = nullptr, const QString &name = QString());
    virtual QAction *createAction(QObject *parent = nullptr, const QString &name = QString());

    void setWorkingDirectory(const QDir &dir);
    QDir workingDirectory() const;

    void setLanguageChangeEnabled(bool enabled);
    bool isLanguageChangeEnabled() const;

    void setTranslationEnabled(bool enabled);
    bool isTranslationEnabled() const;

    QString errorString() const;

private:
    QScopedPointer<QUiLoaderPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QUiLoader)
    Q_DISABLE_COPY_MOVE(QUiLoader)
};

QT_END_NAMESPACE

#endif // QUILOADER_H
