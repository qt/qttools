// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


#include "translationsettingsdialog.h"
#include "messagemodel.h"
#include "phrase.h"

#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

TranslationSettingsDialog::TranslationSettingsDialog(QWidget *parent)
  : QDialog(parent)
{
    m_ui.setupUi(this);

    for (int i = QLocale::C + 1; i < QLocale::LastLanguage; ++i) {
        const auto language = QLocale::Language(i);
        QString lang = QLocale::languageToString(language);
        const auto loc = QLocale(language);
        // Languages for which we have no data get mapped to the default locale;
        // its endonym is unrelated to the language requested. For English, the
        // endonym is the name we already have; don't repeat it.
        if (loc.language() == language && language != QLocale::English) {
            const QString native = loc.nativeLanguageName();
            if (!native.isEmpty()) //: <english> (<endonym>)  (language names)
                lang = tr("%1 (%2)").arg(lang, native);
        }
        m_ui.srcCbLanguageList->addItem(lang, QVariant(i));
    }
    m_ui.srcCbLanguageList->model()->sort(0, Qt::AscendingOrder);
    m_ui.srcCbLanguageList->insertItem(0, QLatin1String("POSIX"), QVariant(QLocale::C));

    m_ui.tgtCbLanguageList->setModel(m_ui.srcCbLanguageList->model());
}

void TranslationSettingsDialog::setDataModel(DataModel *dataModel)
{
    m_dataModel = dataModel;
    m_phraseBook = 0;
    QString fn = QFileInfo(dataModel->srcFileName()).baseName();
    setWindowTitle(tr("Settings for '%1' - Qt Linguist").arg(fn));
}

void TranslationSettingsDialog::setPhraseBook(PhraseBook *phraseBook)
{
    m_phraseBook = phraseBook;
    m_dataModel = 0;
    QString fn = QFileInfo(phraseBook->fileName()).baseName();
    setWindowTitle(tr("Settings for '%1' - Qt Linguist").arg(fn));
}

static void fillTerritoryCombo(const QVariant &lng, QComboBox *combo)
{
    combo->clear();
    QLocale::Language lang = QLocale::Language(lng.toInt());
    if (lang != QLocale::C) {
        const auto matches = QLocale::matchingLocales(lang, QLocale::AnyScript,
                                                      QLocale::AnyTerritory);
        for (const auto &loc : matches) {
            QString name = QLocale::territoryToString(loc.territory());
            if (loc.language() != QLocale::English) {
                QString endonym = loc.nativeTerritoryName();
                if (!endonym.isEmpty())
                    name = TranslationSettingsDialog::tr("%1 (%2)").arg(name, endonym);
            }
            combo->addItem(name, QVariant(loc.territory()));
        }
        combo->model()->sort(0, Qt::AscendingOrder);
    }
    combo->insertItem(0, TranslationSettingsDialog::tr("Any Territory"),
                      QVariant(QLocale::AnyTerritory));
    combo->setCurrentIndex(0);
}

void TranslationSettingsDialog::on_srcCbLanguageList_currentIndexChanged(int idx)
{
    fillTerritoryCombo(m_ui.srcCbLanguageList->itemData(idx), m_ui.srcCbCountryList);
}

void TranslationSettingsDialog::on_tgtCbLanguageList_currentIndexChanged(int idx)
{
    fillTerritoryCombo(m_ui.tgtCbLanguageList->itemData(idx), m_ui.tgtCbCountryList);
}

void TranslationSettingsDialog::on_buttonBox_accepted()
{
    int itemindex = m_ui.tgtCbLanguageList->currentIndex();
    QVariant var = m_ui.tgtCbLanguageList->itemData(itemindex);
    QLocale::Language lang = QLocale::Language(var.toInt());

    itemindex = m_ui.tgtCbCountryList->currentIndex();
    var = m_ui.tgtCbCountryList->itemData(itemindex);
    QLocale::Territory territory = QLocale::Territory(var.toInt());

    itemindex = m_ui.srcCbLanguageList->currentIndex();
    var = m_ui.srcCbLanguageList->itemData(itemindex);
    QLocale::Language lang2 = QLocale::Language(var.toInt());

    itemindex = m_ui.srcCbCountryList->currentIndex();
    var = m_ui.srcCbCountryList->itemData(itemindex);
    QLocale::Territory territory2 = QLocale::Territory(var.toInt());

    if (m_phraseBook) {
        m_phraseBook->setLanguageAndTerritory(lang, territory);
        m_phraseBook->setSourceLanguageAndTerritory(lang2, territory2);
    } else {
        m_dataModel->setLanguageAndTerritory(lang, territory);
        m_dataModel->setSourceLanguageAndTerritory(lang2, territory2);
    }

    accept();
}

void TranslationSettingsDialog::showEvent(QShowEvent *)
{
    QLocale::Language lang, lang2;
    QLocale::Territory territory, territory2;

    if (m_phraseBook) {
        lang = m_phraseBook->language();
        territory = m_phraseBook->territory();
        lang2 = m_phraseBook->sourceLanguage();
        territory2 = m_phraseBook->sourceTerritory();
    } else {
        lang = m_dataModel->language();
        territory = m_dataModel->territory();
        lang2 = m_dataModel->sourceLanguage();
        territory2 = m_dataModel->sourceTerritory();
    }

    int itemindex = m_ui.tgtCbLanguageList->findData(QVariant(int(lang)));
    m_ui.tgtCbLanguageList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);

    itemindex = m_ui.tgtCbCountryList->findData(QVariant(int(territory)));
    m_ui.tgtCbCountryList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);

    itemindex = m_ui.srcCbLanguageList->findData(QVariant(int(lang2)));
    m_ui.srcCbLanguageList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);

    itemindex = m_ui.srcCbCountryList->findData(QVariant(int(territory2)));
    m_ui.srcCbCountryList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);
}

QT_END_NAMESPACE
