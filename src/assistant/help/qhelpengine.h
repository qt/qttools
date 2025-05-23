// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef QHELPENGINE_H
#define QHELPENGINE_H

#include <QtHelp/qhelpenginecore.h>

QT_BEGIN_NAMESPACE

class QHelpContentModel;
class QHelpContentWidget;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpEnginePrivate;
class QHelpSearchEngine;

class QHELP_EXPORT QHelpEngine : public QHelpEngineCore
{
    Q_OBJECT

public:
    explicit QHelpEngine(const QString &collectionFile, QObject *parent = nullptr);
    ~QHelpEngine();

    QHelpContentModel *contentModel() const;
    QHelpIndexModel *indexModel() const;

    QHelpContentWidget *contentWidget();
    QHelpIndexWidget *indexWidget();

    QHelpSearchEngine *searchEngine();

private:
    QHelpEnginePrivate *d;

    friend class HelpEngineWrapper;
};

QT_END_NAMESPACE

#endif // QHELPENGINE_H
