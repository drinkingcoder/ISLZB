#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "Type.h"
#include "Object.h"
struct Edge {
    real_type x, dx, dy;
    int idx; //polygon
};

struct EdgePair {
    Edge left, right;
    real_type zl, dzx, dzy;
    void Increment() {
        left.x += left.dx;
        right.x += right.dx;
        zl += dzx*left.dx + dzy;
    }
};

struct IntervalActiveEdge {
    int idx;
    real_type x;
};

struct Plane {
    Vector3 normal;
    real_type d;
};

struct Polygon {
    Plane plane;
    int idx;
    float ill;
    real_type dy;
    bool flag = false;
};

class IntervalScanLineZBuffer {
public:
public:
    IntervalScanLineZBuffer(cv::Size size);
    virtual ~IntervalScanLineZBuffer() {
    }
    void AddObject(Object & object);
    void ProjectObject(const Object & object);
    void ProjectObjects();
    void Draw();
    void Clear();
    void PrintInfo();
    void Rotate(Vector3 axis);

protected:
public:
    cv::Size    size;
    cv::Mat     scene;

    std::vector< std::vector<Polygon> > polygonYList;
    std::vector< std::vector<Edge> >   edgeYList;
    std::vector<Polygon>   activePolygonList;
    std::vector<EdgePair>   activeEdgeList;
    std::vector<Object>     objects;
    int idxbuffer[100000];

    Matrix3 cameraK;
    Matrix4 camera;
    real_type nearPlane, farPlane;
};
