#ifndef CWE3DVIEW_H
#define CWE3DVIEW_H

#include <QWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QCuboidMesh>
#include <QVector3D>
#include <Qt3DCore/QTransform>
#include "Edged3DBox.h"

class CWE3DView : public QWidget
{
    Q_OBJECT
public:
    explicit CWE3DView(QWidget *parent = nullptr);
    void setView(QVector3D buildingSize, QVector3D domainSize, QVector3D domainCenter);

signals:

public slots:

private:
    Qt3DExtras::Qt3DWindow* graphicsWindow;
    Edged3DBox* buildingBox;
    Edged3DBox* domainBox;

    void setup3DView();
    void addBuildingView(Qt3DCore::QEntity* rootEntity);
    void addDomainView(Qt3DCore::QEntity* rootEntity);
    void setCamera(Qt3DCore::QEntity* rootEntity);
    void setLights(Qt3DCore::QEntity* rootEntity);

};

#endif // CWE3DVIEW_H