// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "helpclient.h"
#if QT_CONFIG(process)
#  include "assistantclient.h"
#endif
#include <QtGui/qdesktopservices.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QString HelpClient::designerManualUrl() const
{
    return documentUrl(u"qtdesigner"_s);
}

std::unique_ptr<HelpClient> HelpClient::create(HelpClientType type)
{
    std::unique_ptr<HelpClient> result;
    switch (type) {
    case HelpClientType::Assistant:
#if QT_CONFIG(process)
        result = std::make_unique<AssistantClient>();
        break;
#endif
    case HelpClientType::Web:
        result = std::make_unique<WebHelpClient>();
        break;
    case HelpClientType::Python:
        result = std::make_unique<PythonWebHelpClient>();
        break;
    }
    return result;
}

static constexpr auto webPrefix = "https://doc.qt.io/qt-6/"_L1;
static constexpr auto pythonWebPrefix = "https://doc.qt.io/qtforpython-6/PySide6/"_L1;
static constexpr auto htmlSuffix = ".html"_L1;
static constexpr auto propertySeparator = "::"_L1;

bool WebHelpClient::showPage(const QString &path, QString *errorMessage)
{
    const bool result = QDesktopServices::openUrl(QUrl(path));
    if (!result) {
        *errorMessage = QCoreApplication::translate("WebHelpClient",
                                                    "Unable to open url (%1).").arg(path);
    }
    return result;
}

// C++ "QWidget::geometry" -> "qwidget.html#geometry-prop
QString WebHelpClient::webPage(const QString &identifier)
{
    const auto propPos = identifier.lastIndexOf(propertySeparator);
    const QString widget = propPos != -1 ? identifier.sliced(0, propPos) : identifier;
    QString result = widget.toLower() + htmlSuffix;
    if (propPos != -1)
        result += u'#' + identifier.sliced(propPos + propertySeparator.size()) + "-prop"_L1;
    return result;
}

// C++ "QWidget::geometry" -> // https://doc.qt.io/qt-6/qwidget.html#geometry-prop
bool WebHelpClient::activateIdentifier(const QString &identifier, QString *errorMessage)
{
    const QString url = identifier.startsWith("http"_L1)
        ? identifier : webPrefix + webPage(identifier);
    return showPage(url, errorMessage);
}

QString WebHelpClient::documentUrl(const QString &) const
{
    return webPrefix;
}

// Python: "QWidget::geometry" -> "QWidget.html" (no property anchors)
QString PythonWebHelpClient::webPage(const QString &identifier)
{
    const auto propPos = identifier.lastIndexOf(propertySeparator);
    const QString widget = propPos != -1 ? identifier.sliced(0, propPos) : identifier;
    return widget + htmlSuffix;
}

// Python: "QWidget::geometry" ->
// "https://doc.qt.io/qtforpython-6/PySide6/QtWidgets/QWidget.html"
bool PythonWebHelpClient::activateIdentifier(const QString &identifier, QString *errorMessage)
{
    QString url = identifier;
    if (!identifier.startsWith("http"_L1)) {
        const auto module = identifier.startsWith("QAction"_L1) ? "QtGui"_L1 : "QtWidgets"_L1;
        url = pythonWebPrefix + module + u'/'+ webPage(identifier);
    }
    return showPage(url, errorMessage);
}

QT_END_NAMESPACE
