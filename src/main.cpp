
#include "shading.hpp"
using namespace lightroom;
int main(int argc, char* argv[])
{
    PipelineManager pm;
    auto cp = new Vertex3D(Vector<3>{ 100, 0, 0 });
    pm.useCamara(pm.addCamara(cp, Vector<3>{ -100,0,0 }, Vector<3>{ 0,0,1 }, AngleOfDegrees[78]));
    pm.addGraphObject(GraphObjType::TRIANGLE,
                      std::array<Vertex3D*, 3>{
                        pm.addVertex(Vector<3>(0, -10, 0), Vector<3>(-1, 0, 0), NormalizedColor(0, 1, 1)),
                        pm.addVertex(Vector<3>(0, 10, 0), Vector<3>(-1, 0, 0), NormalizedColor(1, 0, 1)),
                        pm.addVertex(Vector<3>(0, 0, 30), Vector<3>(-1, 0, 0), NormalizedColor(1, 1, 0))});
    pm.addPointLight(pm.addVertex(Vector<3>(100, 0, 0), Vector<3>(0, 0, 0), NormalizedColor(1, 1, 0.5)));
    while (true)
    {
        TransformMixer3D tm;
        tm.rotate(0, 0, 128);
        pm.render();
        cp->apply(tm.getTransformMatrix());
    }
    
    getchar();
}