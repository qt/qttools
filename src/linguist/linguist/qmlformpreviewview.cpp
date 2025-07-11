// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlformpreviewview.h"

#include "messagemodel.h"

#include <QDir>
#include <QQuickWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QQuickItem>
#include <QHash>
#include <QMessageBox>

using namespace Qt::Literals::StringLiterals;

namespace {

void traverseQml(QObject *obj, QHash<QString, QList<QObject *>> &targets)
{
    if (obj) {
        if (QVariant text = obj->property("text"); text.isValid()) {
            if (auto itr = targets.find(text.toString()); itr != targets.end()) {
                itr->append(obj);
                return;
            }
        }
        for (QObject *child : obj->children())
            traverseQml(child, targets);
    }
}

void matchSources(QQuickItem *root, QHash<QString, QList<QObject *>> &targets)
{
    traverseQml(root, targets);
}

QHash<QString, QList<QObject *>> extractSources(DataModel *m, const QString &contextName)
{
    QHash<QString, QList<QObject *>> t;
    GroupItem *ctx = m->findGroup(contextName, TEXTBASED);
    if (ctx)
        for (int j = 0; j < ctx->messageCount(); j++)
            t[ctx->messageItem(j)->text()] = {};

    for (DataModelIterator it(IDBASED, m); it.isValid(); ++it)
        t[it.current()->text()] = {};
    return t;
}
} // namespace

QT_BEGIN_NAMESPACE

QmlFormPreviewView::QmlFormPreviewView(MultiDataModel *dataModel)
    : QQuickWidget(0), m_dataModel(dataModel) {}

bool QmlFormPreviewView::setSourceContext(int model, MessageItem *messageItem)
{
    if (model < 0 || !messageItem) {
        m_lastModel = -1;
        return true;
    }
    const QDir dir = QFileInfo(m_dataModel->srcFileName(model)).dir();
    const QString fileName = QDir::cleanPath(dir.absoluteFilePath(messageItem->fileName()));
    if (m_lastFormName != fileName) {
        m_lastFormName = fileName;

        setAttribute(Qt::WA_TransparentForMouseEvents, !fileName.endsWith(".ui.qml"_L1));

        setSource(QUrl::fromLocalFile(fileName));
        if (!errors().empty()) {
            m_lastError = true;
            return false;
        }

        m_targets = extractSources(m_dataModel->model(model), messageItem->context());
        matchSources(rootObject(), m_targets);

        setResizeMode(SizeViewToRootObject);
        show();
    } else if (m_lastError) {
        return false;
    }
    if (m_lastModel != model) {
        for (DataModelIterator it(IDBASED, m_dataModel->model(model)); it.isValid(); ++it) {
            MessageItem *message = it.current();
            for (QObject *item : std::as_const(m_targets[message->text()]))
                item->setProperty("text", message->translation());
        }
        for (DataModelIterator it(TEXTBASED, m_dataModel->model(model)); it.isValid(); ++it) {
            MessageItem *message = it.current();
            for (QObject *item : std::as_const(m_targets[message->text()]))
                item->setProperty("text", message->translation());
        }
        m_lastModel = model;
    } else {
        for (QObject *item : std::as_const(m_targets[messageItem->text()]))
            item->setProperty("text", messageItem->translation());
    }
    m_lastError = false;
    return true;
}

QT_END_NAMESPACE
