// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <QtWidgets/QTextBrowser>

QT_BEGIN_NAMESPACE
class QHelpEngineCore;
QT_END_NAMESPACE;

class HelpBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    HelpBrowser(QWidget *parent);
    void showHelpForKeyword(const QString &id);

private:
    QVariant loadResource(int type, const QUrl &name) override;

    QHelpEngineCore *m_helpEngine;
};

#endif
