CONFIG += testcase
QT = core testlib

TARGET = tst_generatedOutput

SOURCES += \
    tst_generatedoutput.cpp

# Write relevant Qt include path to a file, to be read in by QDoc
INCLUDEPATH_CONTENT = \
    -I$$[QT_INSTALL_HEADERS] \
    -I$$[QT_INSTALL_HEADERS]/QtCore

macos: INCLUDEPATH_CONTENT += -F$$shell_quote($$[QT_INSTALL_LIBS])
write_file($$absolute_path($$OUT_PWD/qdocincludepaths.inc, $$OUT_PWD), INCLUDEPATH_CONTENT)|error()
