import sys

from PySide6.QtCore import (QCoreApplication, QItemSelection, QLibraryInfo, QLocale, QTranslator,
                            Slot)
from PySide6.QtWidgets import (QAbstractItemView, QApplication, QListWidget, QMainWindow)

try:
    import linguist_rc  # noqa: F401
except ImportError:
    pass


""""
     Misleading indentation (5) for PYSIDE-2379"""


class Window(QMainWindow):
    def __init__(self):
        super().__init__()
        #: File menu
        file_menu = self.menuBar().addMenu(self.tr("&File"))
        #= quit_id
        quit_action = file_menu.addAction(self.tr("Quit"))
        quit_action.setShortcut(self.tr("CTRL+Q"))
        quit_action.triggered.connect(self.close)
        help_menu = self.menuBar().addMenu(self.tr("&Help"))
        about_qt_action = help_menu.addAction(self.tr("About Qt"))
        about_qt_action.triggered.connect(qApp.aboutQt)  # noqa: F821

        self._list_widget = QListWidget()
        self._list_widget.setSelectionMode(QAbstractItemView.MultiSelection)
        self._list_widget.selectionModel().selectionChanged.connect(self.selection_changed)
        self._list_widget.addItem("C++")
        self._list_widget.addItem("Java")
        self._list_widget.addItem("Python")
        self.setCentralWidget(self._list_widget)

        # Run the test slots to verify syntactic correctness
        self.selection_changed(QItemSelection(), QItemSelection())
        self.test_tr_trailing_comma_behavior()
        self.test_translate_trailing_comma_behavior()
        self.window_method()
        self.test_pyside2863()

    @Slot(QItemSelection, QItemSelection)
    def selection_changed(self, selected, deselected):
        count = len(self._list_widget.selectionModel().selectedRows())
        message = self.tr("%n language(s) selected", "", count)
        self.statusBar().showMessage(message)

    def test_tr_trailing_comma_behavior(self):
        self.tr("SOME TR TEXT, NO COMMENT, TRAILING COMMA",)
        self.tr("SOME TR TEXT", "A COMMENT, NO TRAILING COMMA")
        self.tr("EVEN MORE TR TEXT", "A COMMENT WITH PLURALIZATION", 42)
        self.tr("YET MORE TR TEXT", "A COMMENT WITH PLURALIZATION AND A TRAILING COMMA", 42,)
        count = 42
        self.tr("EVEN MORE TR TEXT", "A COMMENT WITH PLURALIZATION/VARIABLE", count)
        self.tr("YET MORE TR TEXT", "A COMMENT WITH PLURALIZATION/VARIABLE AND A TRAILING COMMA",
                count,)

    def test_translate_trailing_comma_behavior(self):
        QCoreApplication.translate("CONTEXT", "SOME TEXT, NO COMMENT, TRAILING COMMA",)
        QCoreApplication.translate("CONTEXT", "SOME TEXT", "A COMMENT, NO TRAILING COMMA")
        QCoreApplication.translate("CONTEXT", "SOME MORE TEXT", "A COMMENT WITH A TRAILING COMMA",)
        QCoreApplication.translate("CONTEXT", "EVEN MORE TEXT", "A COMMENT WITH PLURALIZATION", 42)
        QCoreApplication.translate("CONTEXT", "YET MORE TEXT",
                                   "A COMMENT WITH PLURALIZATION AND A TRAILING COMMA", 42,)

    def test_translate_fail(self):
        QCoreApplication.translate("CONTEXT", "SOME TEXT, SHOULD NOT BE EXTRACTED",
                                   "A COMMENT WITH PLURALIZATION AND A TRAILING COMMA AND GARBAGE",
                                   42, "THIS SHOULD NOT WORK")

    class NestedClass:
        def foo(self):
            msg = self.tr("Nested Message")  # noqa: F841

    def window_method(self):  # PYSIDE-2379, Don't put this into NestedClass
        msg = self.tr("Window Message")
        msg = self.tr(f"An f-string\\")  # noqa: F541
        msg = self.tr(r"A raw strin\g")
        msg = self.tr(r"A raw strin\g""continued\\")
        msg = self.tr(r"A raw string with escaped quote\"bla")  # noqa: F841

    def test_pyside2863(self):
        """PYSIDE-2863, Check whether id and extra comments are correctly associated."""
        #= id_1
        msg = self.tr("msg1")
        #= id_2
        msg = self.tr("msg2")
        #: Extra comment 3
        msg = self.tr("msg3")
        #: Extra comment 4
        msg = self.tr("msg4")
        msg = self.tr("")
        msg = self.tr("prefix\u00A0\u00A0postfix")
        msg = self.tr("before\U000000A0middle\U000000A0after")
        msg = self.tr("before\u00A0\U000000A0middle\x1F\U000000A0\u00A0after")  # noqa: F841


if __name__ == '__main__':
    app = QApplication(sys.argv)

    translator = QTranslator(app)
    for path in QLibraryInfo.paths(QLibraryInfo.LibraryPath.TranslationsPath):
        if translator.load(QLocale(), 'qtbase', '_', path):
            app.installTranslator(translator)
            break
    translator = QTranslator(app)
    if translator.load(QLocale(), 'example', '_', ':/translations'):
        app.installTranslator(translator)

    window = Window()
    window.show()
    sys.exit(app.exec())
