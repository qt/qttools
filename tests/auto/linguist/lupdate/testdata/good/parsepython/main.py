from pathlib import Path
import sys

from PySide6.QtCore import (QItemSelection, QLibraryInfo, QLocale, QTranslator,
                            Qt, Slot)
from PySide6.QtWidgets import (QAbstractItemView, QApplication, QListWidget,
                               QMainWindow, QWidget)


import linguist_rc


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
        about_qt_action.triggered.connect(qApp.aboutQt)

        self._list_widget = QListWidget()
        self._list_widget.setSelectionMode(QAbstractItemView.MultiSelection)
        self._list_widget.selectionModel().selectionChanged.connect(self.selection_changed)
        self._list_widget.addItem("C++")
        self._list_widget.addItem("Java")
        self._list_widget.addItem("Python")
        self.setCentralWidget(self._list_widget)

    @Slot(QItemSelection, QItemSelection)
    def selection_changed(self, selected, deselected):
        count = len(self._list_widget.selectionModel().selectedRows())
        message = self.tr("%n language(s) selected", "", count)
        self.statusBar().showMessage(message)


if __name__ == '__main__':
    app = QApplication(sys.argv)

    path = QLibraryInfo.location(QLibraryInfo.TranslationsPath)
    translator = QTranslator(app)
    if translator.load(QLocale.system(), 'qtbase_', '', path):
        app.installTranslator(translator)
    translator = QTranslator(app)
    path = ':/translations'
    if translator.load(QLocale.system(), 'example_', '', path):
        app.installTranslator(translator)

    window = Window()
    window.show()
    sys.exit(app.exec())
