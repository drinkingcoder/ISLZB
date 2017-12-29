#include "IntervalScanLineZBuffer.h"
//#include "json.hpp"

//using json = nlohmann::json;
using namespace cv;

int main() {
    std::cout << "start read file" << std::endl;
    Object object("bunny.obj");
    std::cout << "finished read file" << std::endl;
    object.Normalize();
    // object.PrintInfo();
    IntervalScanLineZBuffer scene(Size(640, 480));
    scene.AddObject(object);
    scene.ProjectObjects();
    while (1) {
        scene.ProjectObjects();
        scene.Draw();
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
            default: {
                return 0;
            }
        }
    }
}