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

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QHelpContentModel;
class QHelpContentWidget;
class QHelpEngineCore;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpSearchEngine;

class QHelpEnginePrivate : public QObject
{
    Q_OBJECT

public:
    QHelpEnginePrivate(QHelpEngineCore *helpEngineCore);

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
    QHelpEngineCore *m_helpEngineCore;
};

QT_END_NAMESPACE

#endif // QHELPENGINE_P_H
