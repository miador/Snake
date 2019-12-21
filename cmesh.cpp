#include "cmesh.h"
#include "glwidget.h"
#include <qmath.h>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QOpenGLFunctions>

using namespace std;

CMesh::CMesh()
    : m_count(0), m_primitive(0)
{
}

CMesh::~CMesh()
{
    m_vbo.destroy();
    delete m_vao_binder;
}

void CMesh::add(const QVector3D &v, const QVector3D &n)
{
    m_data.append(v.x());
    m_data.append(v.y());
    m_data.append(v.z());
    m_data.append(n.normalized().x());
    m_data.append(n.normalized().y());
    m_data.append(n.normalized().z());
    m_count++;
}

void CMesh::initVboAndVao()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    int dataSize = m_data.size() * int(sizeof(GLfloat));

    m_vao.create(); // creates vertex array object
    m_vao_binder = new QOpenGLVertexArrayObject::Binder(&m_vao); // binds vertex array object
    m_vbo.create(); // creates vertex buffer object
    m_vbo.bind(); // binds vertex buffer object
    m_vbo.allocate(constData(), dataSize); // copies mesh data to vertex buffer object

    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
}

void CMesh::render(GLWidget* glWidget)
{
    m_vao_binder->rebind();
    glWidget->glDrawArrays(m_primitive, 0, vertexCount());
}

void CMesh::quad3(GLfloat x1, GLfloat y1, GLfloat z1,
                 GLfloat x2, GLfloat y2, GLfloat z2,
                 GLfloat x3, GLfloat y3, GLfloat z3,
                 GLfloat x4, GLfloat y4, GLfloat z4)
{
    QVector3D n = QVector3D::normal(QVector3D(x4 - x1, y4 - y1, z4 - z1), QVector3D(x2 - x1, y2 - y1, z2 - z1));

    add(QVector3D(x1, y1, z1), n);
    add(QVector3D(x4, y4, z4), n);
    add(QVector3D(x2, y2, z2), n);

    add(QVector3D(x3, y3, z3), n);
    add(QVector3D(x2, y2, z2), n);
    add(QVector3D(x4, y4, z4), n);
}

void CMesh::generateCube(GLfloat w, GLfloat h, GLfloat d)
{
    quad3(-w/2, -h/2, -d/2,  +w/2, -h/2, -d/2,  +w/2, +h/2, -d/2,  -w/2, +h/2, -d/2);
    quad3(-w/2, -h/2, +d/2,  -w/2, +h/2, +d/2,  +w/2, +h/2, +d/2,  +w/2, -h/2, +d/2);

    quad3(-w/2, -h/2, -d/2,  -w/2, -h/2, +d/2,  +w/2, -h/2, +d/2,  +w/2, -h/2, -d/2);
    quad3(-w/2, +h/2, -d/2,  +w/2, +h/2, -d/2,  +w/2, +h/2, +d/2,  -w/2, +h/2, +d/2);

    quad3(+w/2, -h/2, -d/2,  +w/2, -h/2, +d/2,  +w/2, +h/2, +d/2,  +w/2, +h/2, -d/2);
    quad3(-w/2, -h/2, -d/2,  -w/2, +h/2, -d/2,  -w/2, +h/2, +d/2,  -w/2, -h/2, +d/2);

    m_primitive = GL_TRIANGLES;

    initVboAndVao();
}

void CMesh::generateSphere(float r, int N)
{
    for(int j = 0 ; j <= N ; j++)
    {
        for(int i = 0 ; i < N ; i++)
        {
             float theta1 = 3.14f * float(j) / float(N);
             float theta2 = 3.14f * float(j-1) / float(N);
             float phi = 3.14f * 2 * float(i) / float(N);

             float nx1 = sin(theta1) * cos(phi);
             float nx2 = sin(theta2) * cos(phi);
             float nz1 = sin(theta1) * sin(phi);
             float nz2 = sin(theta2) * sin(phi);
             float ny1 = cos(theta1);
             float ny2 = cos(theta2);

             float x1 = r * nx1;
             float x2 = r * nx2;
             float y1 = r * ny1;
             float y2 = r * ny2;
             float z1 = r * nz1;
             float z2 = r * nz2;

            add(QVector3D(x1, y1, z1), QVector3D(nx1, ny1, nz1));
            add(QVector3D(x2, y2, z2), QVector3D(nx2, ny2, nz2));
        }
    }

    m_primitive = GL_TRIANGLE_STRIP;

    initVboAndVao();
}

void CMesh::generateMeshFromObjFile(QString filename)
{
    QFile file(filename);
    file.open(QFile::OpenMode::enum_type::ReadOnly);

    std::cout << "Loading " << filename.toStdString() << " - " << (file.isOpen() ? "Found!" : "Not Found!") << std::endl;

    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector2D> texCoords;

    bool hasNormals = false;
    bool hasTexCoords = false;

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        line = line.simplified();

        if (line.length() > 0 && line.at(0) != QChar::fromLatin1('#')) {
            QTextStream lineStream(&line, QIODevice::ReadOnly);
            QString token;
            lineStream >> token;

            if (token == QStringLiteral("v")) {
                float x, y, z;
                lineStream >> x >> y >> z;
                vertices.append(QVector3D( x, y, z ));
            } else if (token == QStringLiteral("vt")) {
                float s,t;
                lineStream >> s >> t;
                texCoords.append(QVector2D(s, t));
                hasTexCoords = true;
            } else if (token == QStringLiteral("vn")) {
                float x, y, z;
                lineStream >> x >> y >> z;
                normals.append(QVector3D( x, y, z ));
                hasNormals = true;
            } else if (token == QStringLiteral("f")) {
                while (!lineStream.atEnd()) {
                    QString faceString;
                    lineStream >> faceString;

                    QVector3D v;
                    QVector3D n;
                    QVector2D uv;

                    QStringList indices = faceString.split(QChar::fromLatin1('/'));

                    v = vertices.at(indices.at(0).toInt() - 1);

                    if(hasTexCoords)
                        uv = texCoords.at(indices.at(1).toInt() - 1);

                    if(hasNormals)
                        n = normals.at(indices.at(2).toInt() - 1);

                    add(v, n);
                }
            }
        }
    }

    m_primitive = GL_TRIANGLES;

    initVboAndVao();
}
