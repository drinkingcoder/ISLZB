#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Type.h"

struct Vertex {
    Vector3 pt;
    Vector3 normal;
    real_type ill;
};

struct Face {
    std::vector<int> vIdx, nIdx;
    Vector3 normal;
    real_type ill;
};

class Object {
public:

    Object():pose(Matrix4::Identity()) {
    }

    Object(std::string fileName):pose(Matrix4::Identity()) {
        LoadObject(fileName);
    }
    virtual ~Object() {
    }

    void LoadObject(std::string fileName);
    void GenerateNormalForFace(Face & f);
    void PrintInfo();
    void Normalize();

    std::vector<Vector3> vertices;
    std::vector<Face>   faces;
    std::vector<Vector3>    normals;
    Matrix4 pose;
};