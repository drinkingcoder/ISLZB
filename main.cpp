#include "IntervalScanLineZBuffer.h"
//#include "json.hpp"

//using json = nlohmann::json;
using namespace cv;

int main() {
    std::cout << "start read file" << std::endl;
    Object object("bunny.obj");
    std::cout << "finished read file" << std::endl;
    object.Normalize();
    object.PrintInfo();
    IntervalScanLineZBuffer scene(Size(640, 480));
    scene.AddObject(object);
    scene.ProjectObjects();
    scene.Draw();
    imshow("scene", scene.scene);
    waitKey();
}