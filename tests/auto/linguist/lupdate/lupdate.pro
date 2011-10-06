CONFIG += testcase
QT += testlib

# June 2011: Marked as unstable due to tst_lupdate::good(heuristics) failing
# due to changes in qtbase/mkspecs/features/qt_config.prf.
CONFIG+=insignificant_test

TARGET = tst_lupdate

SOURCES += tst_lupdate.cpp

