#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QFileInfo>
#include <math.h>
#include <iostream>
#include <qstack.h>
#include <algorithm>
using namespace std;

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_program(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    for(auto it = m_meshes.begin() ; it != m_meshes.end(); it++)
        delete it.value();

    cleanup();
}

QSize GLWidget::sizeHint() const
{
    return QSize(1000, 800);
}

void GLWidget::cleanup()
{
    if (m_program == nullptr)
        return;
    makeCurrent();

    delete m_program;
    m_program = nullptr;
    doneCurrent();
}

void GLWidget::qNormalizeAngle(float &angle)
{
    while (angle < 0)
        angle += 360;
    while (angle > 360)
        angle -= 360;
}

void GLWidget::setXRotation(float angle)
{
    qNormalizeAngle(angle);
    if (abs(angle - m_camXRot) > 0.001f) {
        m_camXRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void GLWidget::setYRotation(float angle)
{
    qNormalizeAngle(angle);
    if (abs(angle - m_camYRot) > 0.001f) {
        m_camYRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void GLWidget::setZRotation(float angle)
{
    qNormalizeAngle(angle);
    if (abs(angle - m_camZRot) > 0.001f) {
        m_camZRot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.2f, 0.3f, 1);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "resources/shader.vs");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "resources/shader.fs");
    m_program->bindAttributeLocation("vertex", 0);
    m_program->bindAttributeLocation("normal", 1);
    m_program->link();

    m_program->bind();
    m_projMatrixLoc = m_program->uniformLocation("projMatrix");
    m_viewMatrixLoc = m_program->uniformLocation("viewMatrix");
    m_modelMatrixLoc = m_program->uniformLocation("modelMatrix");
    m_modelColorLoc = m_program->uniformLocation("modelColor");
    m_lightLoc.position = m_program->uniformLocation("light.position");
    m_lightLoc.ambient = m_program->uniformLocation("light.ambient");
    m_lightLoc.diffuse = m_program->uniformLocation("light.diffuse");

    m_meshes.insert("Cube", new CMesh);
    m_meshes["Cube"]->generateCube(1.0f, 1.0f, 1.0f);

    m_meshes.insert("Sphere", new CMesh);
    m_meshes["Sphere"]->generateSphere(0.5f, 24);

    /*m_meshes.insert("Bunny", new CMesh);
    m_meshes["Bunny"]->generateMeshFromObjFile("resources/bunny.obj");*/

    m_program->release();

    m_robotPosition = QVector3D(0.0f, 0.0f, 0.0f);

    m_robotDir=0;
    leftRot = 0;
    leftTurn = 1;
    rightRot =0;
    rightTurn = 1;
    len =0;

    points.push_back(QVector3D(0,0,0));
}


/////////////////////////////////////////////////// redrawing
void GLWidget::paintGL()
{

    timeElapsed = ctime.elapsed(); // captures rendering time in [ms]
    ctime.restart();

    animate(timeElapsed); // updates object animation parameters


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    QStack<QMatrix4x4> worldMatrixStack;

    m_program->bind();

    m_program->setUniformValue(m_lightLoc.position, QVector3D(0.0f, 15.0f, 0.0f));
    m_program->setUniformValue(m_lightLoc.ambient, QVector3D(0.1f, 0.1f, 0.1f));
    m_program->setUniformValue(m_lightLoc.diffuse, QVector3D(0.9f, 0.9f, 0.9f));

    m_camera.setToIdentity();
    m_camera.translate(0, 0, -m_camDistance);
    m_camera.rotate(90, 1, 0, 0);

    m_world.setToIdentity();
    m_world.scale(0.3f, 0.3f, 0.3f);
    m_world.rotate(m_zRot, 0, 0, 1); // rotates object around Z axis (keyboard)





    // Bunny
   /* worldMatrixStack.push(m_world);
        m_world.translate(0.7f, 0.0f, 0.2f);
        m_world.scale(QVector3D(0.1f, 0.1f, 0.1f));
        setTransforms();
        m_program->setUniformValue(m_modelColorLoc,QVector3D(1.0f, 1.0, 1.0));
        m_meshes["Bunny"]->render(this);
    m_world = worldMatrixStack.pop();*/

    // Robot
    wv.push_back(xPos);
    vv.push_back(zPos);

    if(wv.size() > 10000){
        wv.erase(wv.begin());
        vv.erase(vv.begin());
    }


    //qDebug()<< QVector3D(xPos,0,zPos);
    unsigned long long m_size = wv.size();


        if (m_size>100){
           // float xx = wv[m_size-1];
            //float zz = vv[m_size];


            if(wv[m_size-1]>2){
                xPos=std::max(xPos-4, -2.0f);
            }
            else if(wv[m_size-1]<-2){
                xPos = std::min(xPos+4, 2.0f);
            }
            if(vv[m_size-1]>2){
                zPos =std::max(zPos-4, -2.0f);
            }
            else if(vv[m_size-1]<-2){
                zPos=std::min(zPos+4, 2.0f);
            }


            for(int i = 0 ; i < len ; i++){

                if(wv[m_size-10*i-1]>2){
                    wv[m_size-10*i-1]=std::max(wv[m_size-10*i-1]-4, -2.0f);
                }
                else if(wv[m_size-10*i-1]<-2){
                    wv[m_size-10*i-1]=std::min(wv[m_size-10*i-1]+4, 2.0f);
                }
                if(vv[m_size-10*i-1]>2) {
                        vv[m_size-10*i-1]=std::max(vv[m_size-10*i-1]-4, -2.0f);
                }
                else if(vv[m_size-10*i-1]<-2) {
                        vv[m_size-10*i-1]=std::min(vv[m_size-10*i-1]+4, 2.0f);
                }


                float idx = m_size-10*i-1;

                worldMatrixStack.push(m_world);
                           m_world.translate(wv[idx], 0 , vv[idx]);
                           m_world.rotate(m_robotDir,0,1,0);
                           float theta1 = float(i) / 100 * 2 * float(M_PI);
                           m_world.scale(QVector3D(0.1f, 0.1f, 0.1f));
                           setTransforms();
                           m_program->setUniformValue(m_modelColorLoc, QVector3D(cos(theta1) * 0.5f + 0.5f, sin(theta1) * i*0.5f + 0.5f, 0.0));
                           m_meshes["Sphere"]->render(this);
                m_world = worldMatrixStack.pop();

            }




            for(int i = 1 ; i < len  ; i++){

                if(wv[m_size-10*i-1]>2){
                    wv[m_size-10*i-1]=std::max(wv[m_size-10*i-1]-4, -2.0f);
                }
                else if(wv[m_size-10*i-1]<-2){
                    wv[m_size-10*i-1]=std::min(wv[m_size-10*i-1]+4, 2.0f);
                }
                if(vv[m_size-10*i-1]>2) {
                        vv[m_size-10*i-1]=std::max(vv[m_size-10*i-1]-4, -2.0f);
                }
                else if(vv[m_size-10*i-1]<-2) {
                        vv[m_size-10*i-1]=std::min(vv[m_size-10*i-1]+4, 2.0f);
                }


                float idx = m_size-10*i-1;

                QVector3D P = QVector3D(wv[idx], 0 , vv[idx]);
                QVector3D V = QVector3D(xPos, 0 , zPos) - P;
                //qDebug()<<V.length();
                if(V.length()<0.05){
                    //len++;
                    qDebug()<<"END OF THE GAME";
                    QCoreApplication::quit();

                    //points.erase(points.begin()+i);
                }
            }

            /*for(int i = 0 ; i < 40+len  ; i++){

                if(wv[m_size-10*i-1]>2){
                    wv[m_size-10*i-1]=std::max(wv[m_size-10*i-1]-4, -2.0f);
                }
                else if(wv[m_size-10*i-1]<-2){
                    wv[m_size-10*i-1]=std::min(wv[m_size-10*i-1]+4, 2.0f);
                }

                if(vv[m_size-10*i-1]>2) {
                        vv[m_size-10*i-1]=std::max(vv[m_size-10*i-1]-4, -2.0f);
                }


                else if(vv[m_size-10*i-1]<-2) {
                        vv[m_size-10*i-1]=std::min(vv[m_size-10*i-1]+4, 2.0f);
                }



                worldMatrixStack.push(m_world);
                           m_world.translate(wv[m_size-10*i-1], 0 , vv[m_size-10*i-1]);
                           m_world.rotate(m_robotDir,0,1,0);
                           float theta1 = float(i) / 100 * 2 * float(M_PI);

                           m_world.scale(QVector3D(0.1f, 0.1f, 0.1f));
                           setTransforms();
                           m_program->setUniformValue(m_modelColorLoc, QVector3D(cos(theta1) * 0.5f + 0.5f, sin(theta1) * i*0.5f + 0.5f, 0.0));
                           m_meshes["Sphere"]->render(this);
                       m_world = worldMatrixStack.pop();

            }*/



}





        for (int i=0; i<points.size(); i++){

            worldMatrixStack.push(m_world);
                        m_world.translate(points[i]);
                        m_world.scale(QVector3D(0.1f, 0.1f, 0.1f));
                        setTransforms();
                        m_program->setUniformValue(m_modelColorLoc, QVector3D(1,1,1));
                        m_meshes["Sphere"]->render(this);
                    m_world = worldMatrixStack.pop();
        }



        if((m_timer%400) == 0){

           float x = float(rand()%3801)/1000.0f-1.9f;
           float z = float(rand()%3801)/1000.0f-1.9f;
           points.push_back(QVector3D(x,0,z));
        }

        for (int i=0; i<points.size();){

            QVector3D P = points[i];
            QVector3D V =QVector3D(xPos, 0 , zPos)-P;
            if(V.length()<0.1){
                len++;
                qDebug()<<"collision";
                points.erase(points.begin()+i);
            }
            else {
                 i++;
            }
        }

        /*for(int i = 10; i < len+2; i++){

            if(wv[m_size-10*i-1]>2){
                wv[m_size-10*i-1]=std::max(wv[m_size-10*i-1]-4, -2.0f);
            }
            else if(wv[m_size-10*i-1]<-2){
                wv[m_size-10*i-1]=std::min(wv[m_size-10*i-1]+4, 2.0f);
            }

            if(vv[m_size-10*i-1]>2) {
                    vv[m_size-10*i-1]=std::max(vv[m_size-10*i-1]-4, -2.0f);
            }

            else if(vv[m_size-10*i-1]<-2) {
                    vv[m_size-10*i-1]=std::min(vv[m_size-10*i-1]+4, 2.0f);
            }

            QVector3D P = QVector3D(wv[m_size-10*i-1], 0 , vv[m_size-10*i-1]);
            QVector3D V = QVector3D(xPos, 0 , zPos)-P;
            qDebug()<<V.length();
            if(V.length()<0.01){
                //len++;
                qDebug()<<"END";

                //points.erase(points.begin()+i);
            }

}*/

        worldMatrixStack.push(m_world);
        m_world.translate(QVector3D(2,0,0));
        m_world.scale(QVector3D(0.05f, 0.05f, 4));
        setTransforms();
        m_program->setUniformValue(m_modelColorLoc, QVector3D(1, 1, 1));
        m_meshes["Cube"]->render(this);
    m_world = worldMatrixStack.pop();

    worldMatrixStack.push(m_world);
        m_world.translate(QVector3D(-2,0,0));
        m_world.scale(QVector3D(0.05f, 0.05f, 4));
        setTransforms();
        m_program->setUniformValue(m_modelColorLoc, QVector3D(1, 1, 1));
        m_meshes["Cube"]->render(this);
    m_world = worldMatrixStack.pop();

    worldMatrixStack.push(m_world);
       // m_world.rotate(90, 1, 0, 0);
        m_world.translate(QVector3D(0,0,-2));
        m_world.scale(QVector3D(4, 0.05f, 0.05f));
        setTransforms();
        m_program->setUniformValue(m_modelColorLoc, QVector3D(1, 1, 1));
        m_meshes["Cube"]->render(this);
    m_world = worldMatrixStack.pop();

    worldMatrixStack.push(m_world);
       // m_world.rotate(90, 1, 0, 0);
        m_world.translate(QVector3D(0,0,2));
        m_world.scale(QVector3D(4, 0.05f, 0.05f));
        setTransforms();
        m_program->setUniformValue(m_modelColorLoc, QVector3D(1, 1, 1));
        m_meshes["Cube"]->render(this);
    m_world = worldMatrixStack.pop();

    m_program->release();

    m_timer++;

    update();
}

void GLWidget::setTransforms(void)
{
    m_program->setUniformValue(m_projMatrixLoc, m_proj);
    m_program->setUniformValue(m_viewMatrixLoc, m_camera);
    m_program->setUniformValue(m_modelMatrixLoc, m_world);
}

void GLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(60.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(m_camXRot + 0.5f * dy);
        setYRotation(m_camYRot + 0.5f * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(m_camXRot + 0.5f * dy);
        setZRotation(m_camZRot + 0.5f * dx);
    }
    m_lastPos = event->pos();
}

void GLWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        exit(0);
    else if (e->key() == Qt::Key_Q)
        exit(0);
    else
        QWidget::keyPressEvent(e);

    if(e->key() >= 0 && e->key() <= 255)
        m_keyState[e->key()] = true;


  //a  cout << e->key() << endl;

}

void GLWidget::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() >= 0 && e->key() <= 255 && e->key() != Qt::Key_A && e->key() != Qt::Key_D && e->key() != Qt::Key_W)
        m_keyState[e->key()] = false;
}




/////////////////////////////////////////////////// animation (called from paintGL())
void GLWidget::animate(int timeElapsed)
{
    float dd;

    // zooming
    if(m_keyState[Qt::Key_Z]) m_camDistance += 0.005f;
    if(m_keyState[Qt::Key_X]) m_camDistance -= 0.005f;


    // robot movement
    if(m_keyState[Qt::Key_W]){
        dd = ((float)timeElapsed * 1.0f / 10000.0f);
        xPos += -0.01 * sin(m_robotDir * (2*3.14/360));
        zPos += -0.01 * cos(m_robotDir * (2*3.14/360));
        //m_robotPosition.setZ(m_robotPosition.z() - dd);
    }
    if(m_keyState[Qt::Key_S]){
        dd = ((float)timeElapsed * 1.0f / 1000.0f);

    }
    if(m_keyState[Qt::Key_D]){

        dd = ((float)timeElapsed * 100.0f / 1000.0f);

       m_robotDir -= dd;

       rightRot = rightRot + dd;
       if(rightRot >= 90) {
            m_keyState[Qt::Key_D] = false;

            rightRot = 0;

       }

    }
    if(m_keyState[Qt::Key_A]){
        dd = ((float)timeElapsed * 100.0f / 1000.0f);

       m_robotDir += dd;

       leftRot = leftRot + dd;
       if(leftRot > 90) {
            m_keyState[Qt::Key_A] = false;
            //m_robotDir = 90*leftTurn;
            leftTurn++;
            leftRot = 0;
       }

    }

}
