// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ERRORSVIEW_H
#define ERRORSVIEW_H

#include <QListView>

QT_BEGIN_NAMESPACE

class QStandardItemModel;

class MultiDataModel;

class ErrorsView : public QListView
{
    Q_OBJECT
public:
    enum ErrorType {
        SuperfluousAccelerator,
        MissingAccelerator,
        SurroundingWhitespaceDiffers,
        PunctuationDiffers,
        IgnoredPhrasebook,
        PlaceMarkersDiffer,
        NumerusMarkerMissing
    };

    ErrorsView(MultiDataModel *dataModel, QWidget *parent = 0);
    void clear();
    void addError(int model, const ErrorType type, const QString &arg = QString());
    QString firstError();
private:
    void addError(int model, const QString &error);
    QStandardItemModel *m_list;
    MultiDataModel *m_dataModel;
};

QT_END_NAMESPACE

#endif // ERRORSVIEW_H
