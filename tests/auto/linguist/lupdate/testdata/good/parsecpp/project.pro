SOURCES += main.cpp
SOURCES += finddialog.cpp
SOURCES += excluded.cpp

TR_EXCLUDE = $$PWD/excluded.*

TRANSLATIONS = project.ts
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
