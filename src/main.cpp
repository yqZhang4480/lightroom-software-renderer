#include "shading.hpp"
#include <iostream>
using namespace lightroom;
using namespace std;
struct LockArgs
{
    size_t frameIndex;
    const size_t& maxFps;
    LARGE_INTEGER* const timers;
    const LARGE_INTEGER& timerFreq;
};
double lockFps(LockArgs& LockArgs)
{
    PLARGE_INTEGER pTimerA = &(LockArgs.timers[LockArgs.frameIndex % 2]),
        pTimerB = &(LockArgs.timers[(LockArgs.frameIndex % 2 + 1) % 2]);
    QueryPerformanceCounter(pTimerB);
    double ticks = (double)(pTimerB->QuadPart - pTimerA->QuadPart) * 1000;
    while (LockArgs.maxFps && ticks * LockArgs.maxFps < 999 * LockArgs.timerFreq.QuadPart)
    {
        _ASSERT(ticks > 0);
        QueryPerformanceCounter(pTimerB);
        ticks = (double)(pTimerB->QuadPart - pTimerA->QuadPart) * 1000;
    }
    ++LockArgs.frameIndex;
    return ticks;
}

int main(int argc, char* argv[])
{
    PipelineManager pm;
    auto cp = new Vertex3D(Vector<3>{ 100, 0, 0 });
    pm.useCamara(pm.addCamara(cp, Vector<3>{ -100, 0, 0 }, Vector<3>{ 0, 0, 1 }, AngleOfDegrees[78]));
    pm.addGraphObject<GraphObjType::LINE>(std::array<Vertex3D*, 2>{
        pm.addVertex(Vector<3>(0, -10, 0), Vector<3>(-1, 0, 0), NormalizedColor(0, 1, 1)),
        pm.addVertex(Vector<3>(0, 10, 50), Vector<3>(-1, 0, 0), NormalizedColor(1, 0, 1))
    });
    pm.addPointLight(pm.addVertex(Vector<3>(100, 0, 0), Vector<3>(0, 0, 0), NormalizedColor(1, 1, 0.5)));

    LARGE_INTEGER timers[2]{}, perfFreq{ 0 };
    QueryPerformanceFrequency(&perfFreq);
    int lockFPS = 60;
    LockArgs lockArgs{ 0, lockFPS, timers, perfFreq };
    while (true)
    {
        if (GetAsyncKeyState(VK_F4))
        {
            return 0;
        }
        auto fi = lockArgs.frameIndex;
        double deltapc = lockFps(lockArgs);

        cout << "FPS: " << (int)(1000 * perfFreq.QuadPart / deltapc) << " (lock: " << lockFPS << ")" << endl;

        TransformMixer3D tm;
        tm.rotate(0, 0, 64);
        pm.render();
        //cp->apply(tm.getTransformMatrix());

    }
}