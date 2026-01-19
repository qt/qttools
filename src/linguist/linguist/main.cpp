// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"
#include <helpclient.h>

#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcommandlineoption.h>

#include <QtWidgets/QApplication>
#include <QtGui/QPixmap>

#ifdef Q_OS_MAC
#include <QtCore/QUrl>
#include <QtGui/QFileOpenEvent>
#endif // Q_OS_MAC

QT_USE_NAMESPACE

using namespace Qt::Literals::StringLiterals;

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
    QCoreApplication::setApplicationVersion(QLatin1StringView(qVersion()));
    QCoreApplication::setApplicationName(u"Qt Linguist"_s);
    QApplication::setOverrideCursor(Qt::WaitCursor);

#ifdef Q_OS_MAC
    ApplicationEventFilter eventFilter;
    app.installEventFilter(&eventFilter);
#endif // Q_OS_MAC


    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName() + "\n\n"_L1 + MainWindow::description());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    const QCommandLineOption resourceDirOption(u"resourcedir"_s,
                                               u"Resource directory"_s,
                                               u"directory"_s);
    parser.addOption(resourceDirOption);

    const QCommandLineOption webHelpOption(u"web-help"_s, u"Use the Web documentation"_s);
    parser.addOption(webHelpOption);

    parser.addPositionalArgument(u"files"_s, u"The .ts files to open."_s);

    parser.process(app);

    QString resourceDir = parser.isSet(resourceDirOption)
            ? parser.value(resourceDirOption) : QLibraryInfo::path(QLibraryInfo::TranslationsPath);

    QTranslator translator;
    QTranslator qtTranslator;
    if (translator.load(QLocale(), "linguist"_L1, "_"_L1, resourceDir)) {
        app.installTranslator(&translator);
        if (qtTranslator.load(QLocale(), "qt"_L1, "_"_L1, resourceDir))
            app.installTranslator(&qtTranslator);
        else
            app.removeTranslator(&translator);
    }

    app.setOrganizationName("QtProject"_L1);
    app.setApplicationName("Linguist"_L1);

    MainWindow mw(parser.isSet(webHelpOption) ? HelpClientType::Web : HelpClientType::Assistant);
#ifdef Q_OS_MAC
    eventFilter.setMainWindow(&mw);
#endif // Q_OS_MAC
    app.installEventFilter(&mw);
    mw.show();
    QApplication::restoreOverrideCursor();

    mw.openFiles(parser.positionalArguments());

    return app.exec();
}

#ifdef Q_OS_MAC
#include "main.moc"
#endif // Q_OS_MAC
