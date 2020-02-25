#include <iostream>

#include "mainwindow.h"
#include "window.h"

using namespace std;

MainWindow::MainWindow()
{  

    setWindowTitle(tr("3D Snake Game"));
    setCentralWidget(new Window(this));
    setWindowIcon(QIcon("C:\\Users\\yusufkaradag2\\Desktop\\ss.png"));
}





