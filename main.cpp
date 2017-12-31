#include "IntervalScanLineZBuffer.h"
#include "json.hpp"
#include <fstream>
#include <string>

using json = nlohmann::json;
using namespace cv;
using namespace std;

struct Configuration {
    string obj;
    bool zBuffer;
    bool intervalZBuffer;
};

void ParseConfiguration(Configuration & config) {
    ifstream ifs;
    ifs.open("config.json");
    assert(ifs.is_open());

    json j;
    ifs >> j;
    config = Configuration {
        j["obj"].get<string>(),
        j["zBuffer"].get<bool>(),
        j["intervalZBuffer"].get<bool>()
    };
    cout << "obj = " << config.obj << endl;
    cout << "zBuffer = " << config.zBuffer << endl;
    cout << "intervalZBuffer = " << config.intervalZBuffer << endl;
}

int main() {
    cout << "OpenCV version : " << CV_VERSION << endl;
    cout << "Major version : " << CV_MAJOR_VERSION << endl;
    cout << "Minor version : " << CV_MINOR_VERSION << endl;
    cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;

    Configuration config;
    ParseConfiguration(config);
    assert(config.zBuffer^config.intervalZBuffer);

    std::cout << "start read obj" << std::endl;
    Object object(config.obj);
    std::cout << "finished read file" << std::endl;
    object.Normalize();
    // object.PrintInfo();
    IntervalScanLineZBuffer scene(Size(640, 480));
    scene.AddObject(object);
    scene.ProjectObjects();
    while (1) {
        scene.ProjectObjects();
        if (config.zBuffer) {
            scene.RawScanLineZBufferDraw();
        } else {
            scene.Draw();
        }
        char c = waitKey();
        switch (c) {
            case 'q': {
                scene.Rotate(Vector3::UnitX());
                break;
            }
            case 'w': {
                scene.Rotate(Vector3::UnitY());
                break;
            }
            case 'e': {
                scene.Rotate(Vector3::UnitZ());
                break;
            }
            case 'Q': {
                scene.Rotate(-Vector3::UnitX());
                break;
            }
            case 'W': {
                scene.Rotate(-Vector3::UnitY());
                break;
            }
            case 'E': {
                scene.Rotate(-Vector3::UnitZ());
                break;
            }
            case 'z': {
                scene.LightRotate(Vector3::UnitX());
                break;
            }
            case 'x': {
                scene.LightRotate(Vector3::UnitY());
                break;
            }
            case 'c': {
                scene.LightRotate(Vector3::UnitZ());
                break;
            }
            case 'Z': {
                scene.LightRotate(-Vector3::UnitX());
                break;
            }
            case 'X': {
                scene.LightRotate(-Vector3::UnitY());
                break;
            }
            case 'C': {
                scene.LightRotate(-Vector3::UnitZ());
                break;
            }
            default: {
                return 0;
            }
        }
    }
}