// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SIGNALSLOTEDITOR_P_H
#define SIGNALSLOTEDITOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <QtCore/qpointer.h>
#include <QtCore/qabstractitemmodel.h>

#include <connectionedit_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class DomConnection;

namespace qdesigner_internal {

class SignalSlotEditor;

class SignalSlotConnection : public Connection
{
public:
    explicit SignalSlotConnection(ConnectionEdit *edit, QWidget *source = nullptr, QWidget *target = nullptr);

    void setSignal(const QString &signal);
    void setSlot(const QString &slot);

    QString sender() const;
    QString receiver() const;
    inline QString signal() const { return m_signal; }
    inline QString slot() const { return m_slot; }

    DomConnection *toUi() const;

    void updateVisibility() override;

    enum State { Valid, ObjectDeleted, InvalidMethod, NotAncestor };
    State isValid(const QWidget *background) const;

    // format for messages, etc.
    QString toString() const;

private:
    QString m_signal, m_slot;
};

class ConnectionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ConnectionModel(QObject *parent = nullptr);
    void setEditor(SignalSlotEditor *editor = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::DisplayRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex connectionToIndex(Connection *con) const;
    Connection *indexToConnection(const QModelIndex &index) const;
    void updateAll();

    const SignalSlotConnection *connectionAt(const QModelIndex &index) const;
    static QString columnText(const SignalSlotConnection *con, int column);

private slots:
    void connectionAdded(qdesigner_internal::Connection *con);
    void connectionRemoved(int idx);
    void aboutToRemoveConnection(qdesigner_internal::Connection *con);
    void aboutToAddConnection(int idx);
    void connectionChanged(qdesigner_internal::Connection *con);

private:
    QPointer<SignalSlotEditor> m_editor;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTEDITOR_P_H
