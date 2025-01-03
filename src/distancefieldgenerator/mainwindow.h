// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

class QLabel;
class QProgressBar;
class DistanceFieldModel;
class QListWidgetItem;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void open(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFont();
    void startProgressBar(quint16 glyphCount);
    void stopProgressBar();
    void updateProgressBar();
    void selectAll();
    void updateSelection();
    void updateUnicodeRanges();
    void populateUnicodeRanges();
    void save();
    void saveAs();
    void displayError(const QString &errorString);
    void selectString();
    void about();

private:
    void setupConnections();
    void writeFile();
    QByteArray createSfntTable();

    Ui::MainWindow *ui;
    QString m_fontDir;
    QString m_fontFile;
    QSettings m_settings;
    DistanceFieldModel *m_model;
    QLabel *m_statusBarLabel;
    QProgressBar *m_statusBarProgressBar;
    QString m_fileName;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H
