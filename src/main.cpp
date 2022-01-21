#include "pipeline.hpp"
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
    auto sm = new SequenceMap(PxCoordinate{ 1920, 1080 });
    PipelineManager<TextureVertex3DIn, TextureVertex3DOut, Line3D, TextureTriangle3D> pm(
        Camara(Vector<3>{ 100, 0, 0 }, Vector<3>{ -100, 0, 0 }, Vector<3>{ 0, 0, 1 }, AngleOfDegrees[78]),
        sm);
    auto texture = new ImageMap(L".\\test.png", PxCoordinate{ 1000, 1000 });
    auto vs = std::vector<TextureVertex3DIn*>{
        new TextureVertex3DIn{ Vector<3>(30, -60, -60), PxCoordinate(0,    1000), texture },
        new TextureVertex3DIn{ Vector<3>(30,  60, -60), PxCoordinate(1000, 1000), texture },
        new TextureVertex3DIn{ Vector<3>(30,  60,  60), PxCoordinate(1000, 0),    texture },
        new TextureVertex3DIn{ Vector<3>(30, -60,  60), PxCoordinate(0,    0),    texture } };

    LARGE_INTEGER timers[2]{}, perfFreq{ 0 };
    QueryPerformanceFrequency(&perfFreq);
    int lockFPS = 200;
    LockArgs lockArgs{ 0, lockFPS, timers, perfFreq };
    while (true)
    {
        pm.clear();
        pm.addGraphObject(PrimitiveInputType::TRIANGLE_FAN, vs);

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
        sm->wipe();
        //tp.apply(tm.getTransformMatrix());

    }
}