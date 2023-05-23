// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"
#include "globals.h"

#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>

#include <QtWidgets/QApplication>
#include <QtGui/QPixmap>

#ifdef Q_OS_MAC
#include <QtCore/QUrl>
#include <QtGui/QFileOpenEvent>
#endif // Q_OS_MAC

QT_USE_NAMESPACE

#ifdef Q_OS_MAC
class ApplicationEventFilter : public QObject
{
    Q_OBJECT

public:
    ApplicationEventFilter()
        : m_mainWindow(0)
    {
    }

    void setMainWindow(MainWindow *mw)
    {
        m_mainWindow = mw;
        if (!m_filesToOpen.isEmpty() && m_mainWindow) {
            m_mainWindow->openFiles(m_filesToOpen);
            m_filesToOpen.clear();
        }
    }

protected:
    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (object == qApp && event->type() == QEvent::FileOpen) {
            QFileOpenEvent *e = static_cast<QFileOpenEvent*>(event);
            QString file = e->url().toLocalFile();
            if (!m_mainWindow)
                m_filesToOpen << file;
            else
                m_mainWindow->openFiles(QStringList() << file);
            return true;
        }
        return QObject::eventFilter(object, event);
    }

private:
    MainWindow *m_mainWindow;
    QStringList m_filesToOpen;
};
#endif // Q_OS_MAC

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setOverrideCursor(Qt::WaitCursor);

#ifdef Q_OS_MAC
    ApplicationEventFilter eventFilter;
    app.installEventFilter(&eventFilter);
#endif // Q_OS_MAC

    QStringList files;
    QString resourceDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    QStringList args = app.arguments();

    for (int i = 1; i < args.size(); ++i) {
        QString argument = args.at(i);
        if (argument == QLatin1String("-resourcedir")) {
            if (i + 1 < args.size()) {
                resourceDir = QFile::decodeName(args.at(++i).toLocal8Bit());
            } else {
                // issue a warning
            }
        } else if (!files.contains(argument)) {
            files.append(argument);
        }
    }

    QTranslator translator;
    QTranslator qtTranslator;
    if (translator.load(QLocale(), QLatin1String("linguist"), QLatin1String("_"), resourceDir)) {
        app.installTranslator(&translator);
        if (qtTranslator.load(QLocale(), QLatin1String("qt"), QLatin1String("_"), resourceDir))
            app.installTranslator(&qtTranslator);
        else
            app.removeTranslator(&translator);
    }

    app.setOrganizationName(QLatin1String("QtProject"));
    app.setApplicationName(QLatin1String("Linguist"));

    MainWindow mw;
#ifdef Q_OS_MAC
    eventFilter.setMainWindow(&mw);
#endif // Q_OS_MAC
    mw.show();
    QApplication::restoreOverrideCursor();

    mw.openFiles(files, true);

    return app.exec();
}

#ifdef Q_OS_MAC
#include "main.moc"
#endif // Q_OS_MAC
