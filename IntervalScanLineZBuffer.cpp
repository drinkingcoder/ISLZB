#include "IntervalScanLineZBuffer.h"

#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <limits>
#include <cassert>

using namespace std;
using namespace cv;
namespace {
inline bool almost_equal(real_type a, real_type b) {
    return abs(a - b) <= numeric_limits<real_type>::epsilon();
}

bool CompareEdge(const Edge & a, const Edge & b) {
    if (a.x < b.x) {
        return true;
    } else if (a.x == b.x && a.dx < b.dx) {
        return true;
    } else {
        return false;
    }
}

bool CompareIntervalActiveEdge(const IntervalActiveEdge & a, const IntervalActiveEdge & b) {
    return a.x < b.x;
}

inline EdgePair FindEdgePair(int idx, const std::vector<Edge> &edges) {
    EdgePair res;
    int flag = 0;
    for (auto itr = edges.begin(); itr != edges.end(); itr++) {
        if (itr->idx != idx) {
            continue;
        }
        if (flag == 0) {
            res.left = *itr;
            flag++;
        } else {
            res.right = *itr;
            flag++;
            break;
        }
    }
    if (flag < 2) {
        cout << "Error: don't find pair of edges for [" << idx << "]" << endl;
    }
    return res;
}

inline void ConstructDepthIncrement(EdgePair & edgePair, const Plane & plane, const int y) {
    if (almost_equal(plane.normal(2), 0)) {
        edgePair.zl = numeric_limits<real_type>::max();
        edgePair.dzx = 0;
        edgePair.dzy = 0;
    } else {
        edgePair.zl = -(plane.d + 
                        plane.normal(0) * edgePair.left.x +
                        plane.normal(1) * y) / plane.normal(2);
        cout << "zl = " << edgePair.zl << endl;
        cout << "d = " << plane.d << endl;
        edgePair.dzx = -(plane.normal(0) / plane.normal(2));
        edgePair.dzy = - plane.normal(1) / plane.normal(2);
    }
}

inline void FindLeftEdge(EdgePair & edgePair, const vector<Edge> & edges) {
    for (auto edgeItr = edges.begin(); edgeItr != edges.end(); edgeItr++) {
        if (edgeItr->idx == edgePair.left.idx) {
            edgePair.left = *edgeItr;
            return;
        }
    }
}

inline void FindRightEdge(EdgePair & edgePair, const vector<Edge> & edges) {
    for (auto edgeItr = edges.rbegin(); edgeItr != edges.rend(); edgeItr++) {
        if (edgeItr->idx == edgePair.left.idx && edgeItr->x > edgePair.left.x) {
            edgePair.right = *edgeItr;
            return;
        }
    }
}
}

IntervalScanLineZBuffer::IntervalScanLineZBuffer(cv::Size _size):
    size(_size),
    scene(size, CV_32S) {

    edgeYList.resize(size.height);
    polygonYList.resize(size.height);

    camera = Matrix4::Identity();
    camera(0, 3) = 320;
    camera(1, 3) = 240;
    camera(2, 3) = 1000;

    activeEdgeList.clear();
    activePolygonList.clear();
    objects.clear();
}

void IntervalScanLineZBuffer::AddObject(Object & object) {
    objects.push_back(object);
}

void IntervalScanLineZBuffer::ProjectObject(const Object &object) {
    const auto & faces = object.faces;
    auto vertices = object.vertices;
    Matrix4 projectMatrix = camera * object.pose;
    cout << "project matrix " << projectMatrix << endl;

    for (auto vItr = 0; vItr != vertices.size(); vItr++) {
        vertices[vItr] = transform(projectMatrix, vertices[vItr]);
    }

    // iterator will result in error
    for (auto fItr = 0; fItr != faces.size(); fItr++) {
        const Face & face = faces[fItr];
        Vector3 normal = rotate(projectMatrix, face.normal);
        const vector<int> & vIdx = face.vIdx;

        Vector3 v1 = vertices[vIdx[vIdx.size() - 1]];
        v1(1) = round(v1(1));
        Vector3 v2 = vertices[vIdx[0]];
        v2(1) = round(v2(1));

        Polygon polygon;
        polygon.idx = fItr;
        real_type max_y = -numeric_limits<real_type>::max(), min_y = numeric_limits<real_type>::max();

        if (v1(1) > v2(1)) {
            swap(v1, v2);
        }
        max_y = max(max_y, v2(1));
        min_y = min(min_y, v1(1));

        Edge edge;
        edge.dy = round(v2(1)) - round(v1(1));
        if (edge.dy > 0) {
            edge.x = v1(0);
            edge.idx = polygon.idx;
            edge.dx = (v2(0) - v1(0)) / (v2(1) - v1(1));
            edgeYList[round(v1(1))].push_back(edge);
        }
        auto vPreIdxItr = vIdx.begin();
        for (auto vIdxItr = vIdx.begin()+1; vIdxItr != vIdx.end(); vIdxItr++, vPreIdxItr++) {
            v1 = vertices[*vPreIdxItr];
            v2 = vertices[*vIdxItr];
            // cout << "vertex after transform " << v1 << endl;
            v2(1) = round(v2(1));
            v1(1) = round(v1(1));
            if (v1(1) > v2(1)) {
                swap(v1,v2);
            }

            Edge edge;
            edge.dy = round(v2(1)) - round(v1(1));
            if (edge.dy == 0) {
                continue;
            }

            edge.x = v1(0);
            edge.idx = polygon.idx;
            edge.dx = (v2(0) - v1(0)) / (v2(1) - v1(1));
            edgeYList[round(v1(1))].push_back(edge);

            max_y = max(max_y, v2(1));
            min_y = min(min_y, v1(1));
        }
        polygon.dy = round(max_y) - round(min_y);
        if (polygon.dy == 0) {
            continue;
        }
        if (max_y < size.height && min_y > 0) {
            Vector3 v = vertices[face.vIdx[0]];
            // cout << "vertex = " << v << endl;
            polygon.plane.normal = normal;
            polygon.plane.d = - normal.dot(v);
            polygon.ill = abs(Vector3::UnitZ().dot(polygon.plane.normal));
            polygonYList[round(min_y)].push_back(polygon);
        }
    }
}

void IntervalScanLineZBuffer::ProjectObjects() {
    edgeYList.clear();
    edgeYList.resize(size.height);
    polygonYList.clear();
    polygonYList.resize(size.height);
    for (auto oItr = objects.begin(); oItr != objects.end(); oItr++) {
        this->ProjectObject(*oItr);
    }
    for_each(edgeYList.begin(), edgeYList.end(), [](vector<Edge>& e) { sort(e.begin(), e.end(), CompareEdge); });
}

void IntervalScanLineZBuffer::Draw() {
    scene = Mat(size, CV_32F, Scalar::all(0));
    activePolygonList.clear();
    activeEdgeList.clear();

    for (auto y = 0; y < size.height; y++) {
        // cout << "y = " << y << endl;
        const auto & polys = polygonYList[y];
        const auto & edges = edgeYList[y];

        // add activated polygons
        for (auto polItr = polys.begin(); polItr != polys.end(); polItr++) {
            activePolygonList.push_back(*polItr);
            auto edgePair = FindEdgePair(polItr->idx, edges);
            ConstructDepthIncrement(edgePair, polItr->plane, y);
            activeEdgeList.push_back(edgePair);
        }

        // contruct interval edge list
        vector<IntervalActiveEdge> intervalActiveEdgeList;
        // intervalActiveEdgeList.reserve(activeEdgeList.size()*2);
        for (auto edgeItr = activeEdgeList.begin(); edgeItr != activeEdgeList.end(); edgeItr++) {
            IntervalActiveEdge ae;
            ae.idx = distance(activeEdgeList.begin(), edgeItr);
            ae.x = edgeItr->left.x;
            intervalActiveEdgeList.push_back(ae);
            ae.x = edgeItr->right.x;
            intervalActiveEdgeList.push_back(ae);
        }
        // cout << "sort " << endl;
        // cout << "active edge list size = " << activeEdgeList.size() << endl;
        // cout << "interval active edge list size = " << intervalActiveEdgeList.size() << endl;
        sort(intervalActiveEdgeList.begin(), intervalActiveEdgeList.end(), CompareIntervalActiveEdge);

        // cout << "scan " << endl;
        // scan this row
        vector<int> rowList;
        int idx = 0;
        for (auto aeItr = intervalActiveEdgeList.begin(); aeItr != intervalActiveEdgeList.end(); aeItr++, idx++) {
            // cout << "x = " << aeItr->x << endl;
            if (activePolygonList[aeItr->idx].flag == false) {
                // insert the polygon
                // cout << "insert " << aeItr->idx << endl;
                activePolygonList[aeItr->idx].flag = true;
                rowList.push_back(aeItr->idx);
            } else {
                // erase the polygon
                // cout << "erase " << aeItr->idx << endl;
                activePolygonList[aeItr->idx].flag = false;
                for (auto itr = rowList.begin(); itr != rowList.end(); itr++) {
                    if (*itr == aeItr->idx) {
                        rowList.erase(itr);
                        break;
                    }
                }
            }
            real_type minz = numeric_limits<real_type>::max();
            idxbuffer[idx] = -1;
            for (auto itr = rowList.begin(); itr != rowList.end(); itr++) {
                if (minz > activeEdgeList[*itr].zl) {
                    minz = activeEdgeList[*itr].zl;
                    idxbuffer[idx] = *itr;
                }
            }
        }

        // cout << "fill buffer " << endl;
        for (idx = 0; idx < static_cast<int>(intervalActiveEdgeList.size()) - 1; idx++) {
            int left = round(intervalActiveEdgeList[idx].x);
            int right = round(intervalActiveEdgeList[idx + 1].x);
            // cout << "left = " << left << endl;
            // cout << "right = " << right << endl;
            if (idxbuffer[idx] == -1) {
                continue;
            }
            for (auto i = left; i <= right; i++) {
                scene.at<float>(y, i) = activePolygonList[idxbuffer[idx]].ill;
            }
        }

        cout << "post processing " << endl;
        // postprocessing
        auto polItr = activePolygonList.begin();
        for (auto edgeItr = activeEdgeList.begin(); edgeItr != activeEdgeList.end();) {
            polItr->dy--;
            if (polItr->dy < 0) {
                edgeItr = activeEdgeList.erase(edgeItr);
                polItr = activePolygonList.erase(polItr);
                continue;
            }

            if (edgeItr->left.dy == 0) {
                FindLeftEdge(*edgeItr, edges);
            }
            edgeItr->left.dy--;

            if (edgeItr->right.dy == 0) {
                FindRightEdge(*edgeItr, edges);
            }
            edgeItr->right.dy--;
            edgeItr->Increment();

            edgeItr++;
            polItr++;
        }
    }
    imshow("scene", scene);
}
/*
void IntervalScanLineZBuffer::Draw() {
    scene = Mat(size, CV_32F, Scalar::all(0));
    activeEdgeList.clear();
    activePolygonList.clear();
    for (auto y = 0; y < size.height; y++) {
        cout << "y = " << y << endl;
        //preprocessing
        auto zbuffer = Mat(1, size.width, CV_32F, Scalar::all(numeric_limits<float>::max()));
        const auto & polys = polygonYList[y];
        const auto & edges = edgeYList[y];
        cout << "edges" << endl;
        for (auto itr = edges.begin(); itr != edges.end(); itr++) {
            cout << itr->x << " ";
        }
        cout << endl;
        for (auto polItr = polys.begin(); polItr != polys.end(); polItr++) {
            activePolygonList.push_back(*polItr);
            auto edgePair = FindEdgePair(polItr->idx, edges);
            ConstructDepthIncrement(edgePair, polItr->plane, y);
            activeEdgeList.push_back(edgePair);
        }

        // processing
        auto polItr = activePolygonList.begin();
        for (auto edgeItr = activeEdgeList.begin(); edgeItr != activeEdgeList.end();)
        {
            // draw zbuffer
            auto z = edgeItr->zl;
            for (int p = round(edgeItr->left.x); p <= round(edgeItr->right.x); p++) {
                if (z < zbuffer.at<float>(p)) {
                    zbuffer.at<float>(p) = z;
                    scene.at<float>(y, p) = polItr->ill;
                    if (polItr->ill > 1 || polItr->ill < 0) {
                        cout << "overflow !" << endl;
                        return;
                    }
                }
                z += edgeItr->dzx;
            }

            // postprocessing
            polItr->dy--;
            if (polItr->dy < 0) {
                edgeItr = activeEdgeList.erase(edgeItr);
                polItr = activePolygonList.erase(polItr);
                continue;
            } 

            if (edgeItr->left.dy == 0) {
                FindLeftEdge(*edgeItr, edges);
            }
            edgeItr->left.dy--;

            if (edgeItr->right.dy == 0) {
                FindRightEdge(*edgeItr, edges);
            }
            edgeItr->right.dy--;
            edgeItr->Increment();

            edgeItr++;
            polItr++;
        }
    }


    imshow("scene", scene);
}
*/

void IntervalScanLineZBuffer::Clear() {
    edgeYList.clear();
    edgeYList.resize(size.height);
    polygonYList.clear();
    polygonYList.resize(size.height);

    activeEdgeList.clear();
    activePolygonList.clear();
    objects.clear();

    edgeYList.resize(size.height);
    polygonYList.resize(size.height);
}

void IntervalScanLineZBuffer::PrintInfo() {
    for (auto y = 0; y < size.height; y++) {
        cout << "y = " << y << endl;
    }
}

void IntervalScanLineZBuffer::Rotate(Vector3 axis) {
    camera.block(0, 0, 3, 3) = camera.block(0, 0, 3, 3) * Matrix3(Eigen::AngleAxisf(M_PI/10, axis));
}