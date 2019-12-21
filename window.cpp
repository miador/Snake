#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <iostream>

#include "glwidget.h"
#include "window.h"
#include "mainwindow.h"

using namespace std;

Window::Window(MainWindow *mw)
    : mainWindow(mw)
{

    // creates glWidget object, constructor of GLWidget class is called
    glWidget = new GLWidget;

    // sets layout of glWidget
    QWidget *w = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(w);
    QHBoxLayout *container = new QHBoxLayout;
    container->addWidget(glWidget);
    w->setLayout(container);
    setLayout(mainLayout);

}



