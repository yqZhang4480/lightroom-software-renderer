#include "shading.hpp"
using namespace lightroom;
int main(int argc, char* argv[])
{
    PipelineManager pm;
    pm.useCamara(pm.addCamara(Vector<3>{ 100, 0, 0 }, Vector<3>{ -100,0,0 }, Vector<3>{ 0,0,1 }, AngleAsDegree[78]));
    pm.addGraphObject(GraphObjType::TRIANGLE,
                      std::array<Vertex3D*, 3>{
                        pm.addVertex(Vector<3>(0, 0, 0), Vector<3>(-1, 0, 0), NormalizedColor(1, 1, 1)),
                        pm.addVertex(Vector<3>(0, 10, 0), Vector<3>(-1, 0, 0), NormalizedColor(1, 1, 1)),
                        pm.addVertex(Vector<3>(0, 10, 30), Vector<3>(-1, 0, 0), NormalizedColor(1, 1, 1))});
    pm.addPointLight(pm.addVertex(Vector<3>(100, 0, 0), Vector<3>(0, 0, 0), NormalizedColor(1, 1, 0.5)));
    pm.render();
    getchar();
}