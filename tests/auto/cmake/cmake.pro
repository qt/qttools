
# Cause make to do nothing.
TEMPLATE = subdirs

# QtUiTools test fail on Windows/MSVC because it's a static library - QTBUG-27087
win32:CONFIG += insignificant_test
CONFIG += ctest_testcase
