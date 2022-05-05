/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef SOURCECODEVIEW_H
#define SOURCECODEVIEW_H

#include <QDir>
#include <QHash>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE

class SourceCodeView : public QPlainTextEdit
{
    Q_OBJECT
public:
    SourceCodeView(QWidget *parent = 0);
    void setSourceContext(const QString &fileName, const int lineNum);

public slots:
    void setActivated(bool activated);

private:
    void showSourceCode(const QString &fileName, const int lineNum);

    bool m_isActive;
    QString m_fileToLoad;
    int m_lineNumToLoad;
    QString m_currentFileName;

    QHash<QString, QString> fileHash;
};

QT_END_NAMESPACE

#endif // SOURCECODEVIEW_H
