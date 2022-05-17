// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPENGINE_P_H
#define QHELPENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QSqlQuery;

class QHelpEngineCore;
class QHelpDBReader;
class QHelpContentModel;
class QHelpContentWidget;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpSearchEngine;
class QHelpCollectionHandler;
class QHelpFilterEngine;

class QHelpEngineCorePrivate : public QObject
{
    Q_OBJECT

public:
    virtual ~QHelpEngineCorePrivate();

    virtual void init(const QString &collectionFile,
        QHelpEngineCore *helpEngineCore);

    bool setup();

    QHelpCollectionHandler *collectionHandler = nullptr;
    QHelpFilterEngine *filterEngine = nullptr;
    QString currentFilter;
    QString error;
    bool needsSetup = true;
    bool autoSaveFilter = true;
    bool usesFilterEngine = false;
    bool readOnly = true;

protected:
    QHelpEngineCore *q;

private slots:
    void errorReceived(const QString &msg);
};

class QHelpEnginePrivate : public QHelpEngineCorePrivate
{
    Q_OBJECT

public:
    void init(const QString &collectionFile,
        QHelpEngineCore *helpEngineCore) override;

    QHelpContentModel *contentModel = nullptr;
    QHelpContentWidget *contentWidget = nullptr;

    QHelpIndexModel *indexModel = nullptr;
    QHelpIndexWidget *indexWidget = nullptr;

    QHelpSearchEngine *searchEngine = nullptr;

    friend class QHelpContentProvider;
    friend class QHelpContentModel;
    friend class QHelpIndexProvider;
    friend class QHelpIndexModel;

public slots:
    void setContentsWidgetBusy();
    void unsetContentsWidgetBusy();
    void setIndexWidgetBusy();
    void unsetIndexWidgetBusy();

private slots:
    void scheduleApplyCurrentFilter();
    void applyCurrentFilter();

private:
    bool m_isApplyCurrentFilterScheduled = false;

};

QT_END_NAMESPACE

#endif
