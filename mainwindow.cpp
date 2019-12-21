#include <iostream>

#include "mainwindow.h"
#include "window.h"

using namespace std;

MainWindow::MainWindow()
{  

    setWindowTitle(tr("QT GLGame"));
    setCentralWidget(new Window(this));
}





