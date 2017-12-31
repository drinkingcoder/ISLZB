#include "Object.h"
#include <fstream>

using namespace std;

void Object::LoadObject(const string & fileName) {
    ifstream fin(fileName);
    if (!fin.is_open()) {
        cout << "Failed to open file: " << fileName << endl;
        return;
    }
    int fcount = 0, vcount = 0;

    string type;
    while (fin >> type) {
        if (type == "v") {
            Vector3 v;
            real_type x, y, z;
            fin >> x >> y >> z;
            v << x, y, z;
            vertices.push_back(v*10);
            vcount++;
        } else if (type == "vt") {
            float vt;
            fin >> vt >> vt;    //we don't need texture
        } else if (type == "vn") {
            Vector3 vn;
            real_type x, y, z;
            fin >> x >> y >> z;
            vn << x, y, z;
            normals.push_back(vn);
        } else if (type == "f") {
            fcount++;
            Face f;
            int vIdx, nIdx, tmp;
            while (1) {
                char c = fin.get();
                if (c == ' ') {
                    continue;
                } else if (c == '\n' || c == EOF || c == '\r') {
                    break;
                } else {
                    fin.putback(c);
                }

                fin >> vIdx;
                char splitter = fin.get();
                nIdx = 0;

                if (splitter == '/') {
                    splitter = fin.get();
                    if (splitter == '/') {
                        fin >> nIdx;
                    } else {
                        fin.putback(splitter);
                        fin >> tmp; //we don't need texture
                        splitter = fin.get();
                        if (splitter == '/') {
                            fin >> nIdx;
                        } else {
                            fin.putback(splitter);
                        }
                    }
                } else {
                    fin.putback(splitter);
                }

                f.vIdx.push_back(vIdx - 1);
            }

            this->GenerateNormalForFace(f);
            faces.push_back(f);
        }
    }

    fin.close();
    cout << "faces count = " << fcount << endl;
    cout << "vertex count = " << vcount << endl;
    // faces.resize(1);
    pose = Matrix4::Identity();
}

void Object::GenerateNormalForFace(Face & f) {
    if (f.vIdx.size() < 3) {
        cout << "Can't generate normal for face as vertices in the face is " << f.vIdx.size() << endl;
    }
    Vector3 & a = vertices[f.vIdx[0]];
    Vector3 & b = vertices[f.vIdx[1]];
    Vector3 & c = vertices[f.vIdx[2]];
    f.normal = (b - a).cross(c - b).normalized();
}

void Object::PrintInfo() {
    for (auto faceItr = faces.begin(); faceItr != faces.end(); faceItr++) {
        cout << faceItr - faces.begin() << endl;
        cout << "N =\t" << faceItr->normal << endl;
        cout << "Vertex 0\t" << vertices[faceItr->vIdx[0]] << endl;
        cout << "Vertex 1\t" << vertices[faceItr->vIdx[1]] << endl;
        cout << "Vertex 2\t" << vertices[faceItr->vIdx[2]] << endl;
    }
}

void Object::Normalize() {
    real_type maxy, miny, maxz, minz;
    real_type maxx = maxy = maxz = -numeric_limits<real_type>::max();
    real_type minx = miny = minz = numeric_limits<real_type>::max();
    for (auto vItr = vertices.begin(); vItr != vertices.end(); vItr++) {
        if (maxx < (*vItr)[0]) {
            maxx = (*vItr)[0];
        }
        if (maxy < (*vItr)[1]) {
            maxy = (*vItr)[1];
        }
        if (maxz < (*vItr)[2]) {
            maxz = (*vItr)[2];
        }
        if (minx > (*vItr)[0]) {
            minx = (*vItr)[0];
        }
        if (miny > (*vItr)[1]) {
            miny = (*vItr)[1];
        }
        if (minz > (*vItr)[2]) {
            minz = (*vItr)[2];
        }
    }
    real_type scale = 480/max(max((maxx - minx), (maxy - miny)), (maxz - minz)) * 0.8;
    Vector3 center;
    center << (maxx + minx)/2, (maxy + miny)/2, (maxz + minz)/2;
    for (auto vItr = vertices.begin(); vItr != vertices.end(); vItr++) {
        *vItr = (*vItr - center) * scale;
    }

    pose.block(0, 0, 3, 3) = Matrix3(Eigen::AngleAxisf(M_PI , Vector3::UnitX()));
}