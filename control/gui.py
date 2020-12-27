''' GUI '''

import os
import sys
import time
from PyQt5.QtCore import pyqtSignal, Qt
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtWidgets import (
    QWidget, QApplication, QGridLayout, QLabel, QLineEdit, QPushButton)

from control import MainThread


class MainWindow(QWidget):
    closeSignal = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.last = time.time()
        self.closeSignal.connect(self.on_close)

        self.setWindowTitle("Control")
        self.resize(800, 600)

        grid = QGridLayout()
        grid.setSpacing(10)
        self.setLayout(grid)

        self.edit1 = QLineEdit()
        grid.addWidget(self.edit1, 0, 0)

        self.button1 = QPushButton("Send")
        self.button1.clicked.connect(self.on_clicked)
        grid.addWidget(self.button1, 0, 1)

        self.lable1 = QLabel("RGB", self)
        grid.addWidget(self.lable1, 1, 0)
        self.lable2 = QLabel("IR", self)
        grid.addWidget(self.lable2, 1, 1)
        self.lable3 = QLabel("Depth", self)
        grid.addWidget(self.lable3, 2, 0)

        self.socket_thread = MainThread(self)
        self.socket_thread.start()

    def on_clicked(self):
        ''' on_clicked '''
        text = self.edit1.text()
        self.socket_thread.send(text)
        # self.closeSignal.emit()

    def on_close(self):
        ''' to close window '''
        self.close()

    def closeEvent(self, event):
        ''' Qt Close event '''
        self.socket_thread.stop()
        event.accept()
        print("Bye!")

    def set_image(self, img_type, img, second):
        ''' set_image '''
        if img_type == 1:
            qimg = QImage(img.data, img.shape[1],
                      img.shape[0], QImage.Format_RGB888)
            self.lable1.setPixmap(QPixmap.fromImage(qimg))

            # Save image
            output_path = os.path.abspath(r"./data/rgb")
            os.makedirs(output_path, exist_ok=True)
            output_file = os.path.join(output_path, r"%016.4f.jpg" % (second))
            qimg.save(str(output_file))

            # Calc FPS
            now = time.time()
            seconds = now - self.last
            self.last = now
            print("FPS: {0}".format(1 / seconds))
        elif img_type == 2:
            qimg = QImage(img.data, img.shape[1],
                      img.shape[0], QImage.Format_Grayscale8)
            self.lable2.setPixmap(QPixmap.fromImage(qimg))

            # Save image
            output_path = os.path.abspath(r"./data/ir")
            os.makedirs(output_path, exist_ok=True)
            output_file = os.path.join(output_path, r"%016.4f.jpg" % (second))
            qimg.save(str(output_file))
        elif img_type == 3:
            qimg = QImage(img.data, img.shape[1],
                      img.shape[0], QImage.Format_RGB16)
            self.lable3.setPixmap(QPixmap.fromImage(qimg))

    def keyPressEvent(self, event):
        move_speed = 40
        shift_speed = 55
        rotate_speed = 35
        if event.key() == Qt.Key_W:
            self.socket_thread.send('Car+MB%d' % (move_speed))
        elif event.key() == Qt.Key_S:
            self.socket_thread.send('Car+MF%d' % (move_speed))
        elif event.key() == Qt.Key_A:
            self.socket_thread.send('Car+ML%d' % (shift_speed))
        elif event.key() == Qt.Key_D:
            self.socket_thread.send('Car+MR%d' % (shift_speed))
        elif event.key() == Qt.Key_E:
            self.socket_thread.send('Car+MC%d' % (rotate_speed))
        elif event.key() == Qt.Key_Q:
            self.socket_thread.send('Car+MA%d' % (rotate_speed))
        else:
            print('stop')
            self.socket_thread.send('Car+MS')

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()

    sys.exit(app.exec_())
