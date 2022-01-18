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
    //auto tp = Homogeneous<4>(Vector<3>(0, 0, 20));
    PipelineManager pm;


    LARGE_INTEGER timers[2]{}, perfFreq{ 0 };
    QueryPerformanceFrequency(&perfFreq);
    int lockFPS = 200;
    LockArgs lockArgs{ 0, lockFPS, timers, perfFreq };
    auto texture = new ImageMap(L".\\test.png", PxCoordinate{ 1000, 1000 });
    while (true)
    {
        pm.clear();
        pm.useCamara(pm.addCamara(Vector<3>{ 100, 0, 0 }, Vector<3>{ -100, 0, 0 }, Vector<3>{ 0, 0, 1 }, AngleOfDegrees[78]));
        pm.addGraphObject(PrimitiveType::TRIANGLE_FAN, {
            {Vector<3>(30, -25, -20), PxCoordinate(0, 1000)},
            {Vector<3>(30,  25, -20), PxCoordinate(1000, 1000)},
            {Vector<3>( 0,  25,  20), PxCoordinate(1000, 0)},
            {Vector<3>( 0, -25,  20), PxCoordinate(0, 0)}},
            texture);
        pm.addPointLight(Vector<3>(100, 0, 0), Color(1, 1, 0.5));

        if (GetAsyncKeyState(VK_F4))
        {
            return 0;
        }
        auto fi = lockArgs.frameIndex;
        double deltapc = lockFps(lockArgs);

        cout << "FPS: " << (int)(1000 * perfFreq.QuadPart / deltapc) << " (lock: " << lockFPS << ")" << endl;

        //TransformMixer3D tm;
        //tm.translate(0.5, 0, 0);
        pm.render();
        //tp.apply(tm.getTransformMatrix());

    }
}