// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef DPICHOOSER_H
#define DPICHOOSER_H

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QSpinBox;
class QComboBox;

namespace qdesigner_internal {

struct DPI_Entry;

/* Let the user choose a DPI settings */
class DPI_Chooser : public QWidget {
    Q_DISABLE_COPY_MOVE(DPI_Chooser)
    Q_OBJECT

public:
    explicit DPI_Chooser(QWidget *parent = nullptr);
    ~DPI_Chooser();

    void getDPI(int *dpiX, int *dpiY) const;
    void setDPI(int dpiX, int dpiY);

private slots:
    void syncSpinBoxes();

private:
    void setUserDefinedValues(int dpiX, int dpiY);

    struct DPI_Entry *m_systemEntry;
    QComboBox *m_predefinedCombo;
    QSpinBox *m_dpiXSpinBox;
    QSpinBox *m_dpiYSpinBox;
};
}

QT_END_NAMESPACE

#endif // DPICHOOSER_H
