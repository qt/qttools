CONFIG += lrelease embed_translations

QT += widgets

HEADERS = \
    languagechooser.h \
    mainwindow.h

SOURCES = \
    languagechooser.cpp \
    main.cpp \
    mainwindow.cpp

TRANSLATIONS += \
    translations/i18n_ar.ts \
    translations/i18n_cs.ts \
    translations/i18n_de.ts \
    translations/i18n_el.ts \
    translations/i18n_en.ts \
    translations/i18n_eo.ts \
    translations/i18n_fr.ts \
    translations/i18n_it.ts \
    translations/i18n_ja.ts \
    translations/i18n_ko.ts \
    translations/i18n_nb.ts \
    translations/i18n_ru.ts \
    translations/i18n_sv.ts \
    translations/i18n_zh.ts

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/tools/i18n
INSTALLS += target
