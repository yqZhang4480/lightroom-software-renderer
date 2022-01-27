#pragma once
#include <iostream>
#include <set>
#include "../lightroom.hpp"

namespace lightroom
{
    namespace sample
    {
        class ColoredVertex3DIn : public Vertex3DIn
        {
        public:
            Color color;

            ColoredVertex3DIn(const Vector<3>& position,
                              const Color& color) :
                Vertex3DIn(position), color(color) {}
        };
        class ColoredVertex3DOut : public Vertex3DOut
        {
        public:
            Color color;

            ColoredVertex3DOut(const ColoredVertex3DIn* _vin,
                               PrimitiveInputType primitiveType) :
                Vertex3DOut(_vin, primitiveType), color(_vin->color) {}
        };

        class ColoredTriangle3D : public Triangle3D
        {
        public:
            ColoredTriangle3D(const std::array<Vertex3DOut*, 3>& _vs) : Triangle3D(_vs) {}
        protected:
            virtual void putPixel(
                int _x, int _y, Float _alpha, Float _beta, Float _gamma,
                WritableColorMap* _colorMap, DepthBuffer& _depthBuffer) const override
            {
                int _index = _y * _colorMap->getWidth() + _x;

                Float _depth = linearInterpolation<Float>(
                    _alpha, _beta, _gamma,
                    [](const Vertex3DOut* _v)
                    {
                        return _v->position[2];
                    });
                if (_depth <= _depthBuffer[_index])
                {
                    return;
                }

                auto _color = perspectiveInterpolation<Color>(
                    _alpha, _beta, _gamma,
                    [](const Vertex3DOut* _v)
                    {
                        auto _tv = static_cast<const ColoredVertex3DOut*>(_v);
                        return _tv->color;
                    });

                _depthBuffer[_index] = _depth;
                _colorMap->set(_index, _color);
            }
        };

        class ColoredVertexMain
        {
        private:
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
            SequenceMap* output;
            ImageMap* texture;
            std::vector<ColoredVertex3DIn*> vs;
        public:
            ColoredVertexMain() :
                output(new SequenceMap(PxCoordinate{ 1920, 1080 })),
                texture(new ImageMap(L".\\test.png", PxCoordinate{ 1000, 1000 })),
                vs({
                    new ColoredVertex3DIn{ Vector<3>(-20, -20,  20), Color(0,0,1,1) },
                    new ColoredVertex3DIn{ Vector<3>( 20, -20,  20), Color(1,0,1,1) },
                    new ColoredVertex3DIn{ Vector<3>(-20,  20,  20), Color(0,1,1,1) },
                    new ColoredVertex3DIn{ Vector<3>( 20,  20,  20), Color(1,1,1,1) },
                    new ColoredVertex3DIn{ Vector<3>(-20, -20, -20), Color(0,0,0,1) },
                    new ColoredVertex3DIn{ Vector<3>( 20, -20, -20), Color(1,0,0,1) },
                    new ColoredVertex3DIn{ Vector<3>(-20,  20, -20), Color(0,1,0,1) },
                    new ColoredVertex3DIn{ Vector<3>( 20,  20, -20), Color(1,1,0,1) } })
            {
                auto camara = Camara(Vector<3>{ 173, 0, 100 }, Vector<3>{ -173, 0, -100 }, Vector<3>{ -100, 0, 173 }, AngleOfDegrees[78]);

                PipelineManager<ColoredVertex3DIn, ColoredVertex3DOut, Line3D, ColoredTriangle3D> pm(camara, output);

                LARGE_INTEGER timers[2]{}, perfFreq{ 0 };
                QueryPerformanceFrequency(&perfFreq);
                int lockFPS = 200;
                LockArgs lockArgs{ 0, lockFPS, timers, perfFreq };

                POINT cursorPos1, cursorPos2;
                GetCursorPos(&cursorPos1);
                while (true)
                {
                    TransformMixer3D tm;

                    if (GetForegroundWindow() == GetHWnd())
                    {
                        GetCursorPos(&cursorPos2);

                        int dx = cursorPos2.x - cursorPos1.x;
                        cursorPos1 = cursorPos2;

                        tm.rotate(0, 0, -dx * 128);
                    }

                    pm.camara.apply(tm.getTransformMatrix());
                    //pm.camara.lookAt({ 0, 0, 0 });

                    pm.clear();
                    pm.addGraphObject(PrimitiveInputType::TRIANGLE_FAN,
                                      { vs[0], vs[1], vs[2], vs[6], vs[4], vs[5], vs[1] });
                    pm.addGraphObject(PrimitiveInputType::TRIANGLE_STRIP,
                                      { vs[4],vs[5], vs[6], vs[7], vs[2], vs[3], vs[1], vs[7], vs[5]});

                    if (GetAsyncKeyState(VK_RMENU)) // right alt
                    {
                        break;
                    }
                    auto fi = lockArgs.frameIndex;
                    double deltapc = lockFps(lockArgs);

                    std::cout << "FPS: " << (int)(1000 * perfFreq.QuadPart / deltapc) << " (lock: " << lockFPS << ")" << std::endl;

                    pm.render();
                    output->wipe();
                }
            }

            ~ColoredVertexMain()
            {
                delete output;
                delete texture;

                for (auto v : std::set<ColoredVertex3DIn*>(vs.begin(), vs.end()))
                {
                    delete v;
                }
            }
        };


    }
}