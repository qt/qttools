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

ContextItem *getContext(const DataModel *m, const QString &contextName)
{
    for (int i = 0; i < m->contextCount(); i++)
        if (auto ctx = m->contextItem(i); ctx->context() == contextName)
            return ctx;
    Q_UNREACHABLE_RETURN(nullptr);
}

QHash<QString, QList<QObject *>> extractSources(const DataModel *m, const QString &contextName)
{
    QHash<QString, QList<QObject *>> t;
    ContextItem *ctx = getContext(m, contextName);
    for (int j = 0; j < ctx->messageCount(); j++)
        t[ctx->messageItem(j)->text()] = {};
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
            QString errs;
            for (const auto &error : errors())
                errs += error.toString() + "\n"_L1;
            QMessageBox::warning(this, tr("Qt Linguist"),
                                 tr("Error loading QML file: %1").arg(errs));
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
        ContextItem *ctx = getContext(m_dataModel->model(model), messageItem->context());
        for (int i = 0; i < ctx->messageCount(); i++) {
            MessageItem *message = ctx->messageItem(i);
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
