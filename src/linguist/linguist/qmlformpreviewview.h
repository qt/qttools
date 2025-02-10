// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLFORMPREVIEWVIEW_H
#define QMLFORMPREVIEWVIEW_H

#include <QQuickWidget>

QT_BEGIN_NAMESPACE

class QQuickWidget;
class MessageItem;
class MultiDataModel;

class QmlFormPreviewView : public QQuickWidget
{
    Q_OBJECT
public:
    explicit QmlFormPreviewView(MultiDataModel *dataModel);
    bool setSourceContext(int model, MessageItem *messageItem);

private:
    MultiDataModel *m_dataModel;
    QHash<QString, QList<QObject *>> m_targets;
    QString m_lastFormName;
    int m_lastModel;
    bool m_lastError = false;
};

QT_END_NAMESPACE

#endif // QMLFORMPREVIEWVIEW_H
