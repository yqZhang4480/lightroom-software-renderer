#pragma once
#include <iostream>
#include <set>
#include "../lightroom.hpp"

namespace lightroom
{
    namespace sample
    {
        class TextureVertex3DIn : public Vertex3DIn
        {
        public:
            UVCoordinate uvPosition;
            ColorMap* texture;

            TextureVertex3DIn(const Vector<3>& position,
                              const UVCoordinate& uvPosition, ColorMap* texture) :
                Vertex3DIn(position), uvPosition(uvPosition), texture(texture) {}
        };
        class TextureVertex3DOut : public Vertex3DOut
        {
        public:
            UVCoordinate uvPosition;
            ColorMap* texture;

            TextureVertex3DOut(const TextureVertex3DIn* _vin,
                               PrimitiveInputType primitiveType) :
                Vertex3DOut(_vin, primitiveType),
                uvPosition(_vin->uvPosition), texture(_vin->texture) {}
        };

        class TextureTriangle3D : public Triangle3D
        {
        public:
            TextureTriangle3D(const std::array<Vertex3DOut*, 3>& _vs) : Triangle3D(_vs) {}
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

                auto __u = perspectiveInterpolation<Float>(
                    _alpha, _beta, _gamma,
                    [](const Vertex3DOut* _v)
                    {
                        auto _tv = static_cast<const TextureVertex3DOut*>(_v);
                        return _tv->uvPosition[0];
                    });
                auto __v = perspectiveInterpolation<Float>(
                    _alpha, _beta, _gamma,
                    [](const Vertex3DOut* _v)
                    {
                        auto _tv = static_cast<const TextureVertex3DOut*>(_v);
                        return _tv->uvPosition[1];
                    });

                _depthBuffer[_index] = _depth;
                _colorMap->set(_index,
                               static_cast<const TextureVertex3DOut*>(_vertices[0])->
                               texture->get(UVCoordinate{ __u, __v }));
            }
        };

        class TextureMain
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
            std::vector<TextureVertex3DIn*> vs;
        public:
            TextureMain() :
                output(new SequenceMap(PxCoordinate{ 1920, 1080 })),
                texture(new ImageMap(L".\\test.png", PxCoordinate{ 1000, 1000 })),
                vs({
                    new TextureVertex3DIn{ Vector<3>(30, -25, -20), UVCoordinate(0, 1), texture },
                    new TextureVertex3DIn{ Vector<3>(30,  25, -20), UVCoordinate(1, 1), texture },
                    new TextureVertex3DIn{ Vector<3>(0,   25,  20), UVCoordinate(1, 0), texture },
                    new TextureVertex3DIn{ Vector<3>(0,  -25,  20), UVCoordinate(0, 0), texture } })
            {
                auto camara = Camara(Vector<3>{ 100, 0, 0 }, Vector<3>{ -100, 0, 0 }, Vector<3>{ 0, 0, 1 }, AngleOfDegrees[78]);
                PipelineManager<TextureVertex3DIn, TextureVertex3DOut, Line3D, TextureTriangle3D> pm(camara, output);

                LARGE_INTEGER timers[2]{}, perfFreq{ 0 };
                QueryPerformanceFrequency(&perfFreq);
                int lockFPS = 200;
                LockArgs lockArgs{ 0, lockFPS, timers, perfFreq };
                while (true)
                {
                    pm.clear();
                    pm.addGraphObject(PrimitiveInputType::TRIANGLE_FAN, vs);

                    if (GetAsyncKeyState(VK_LMENU)) // left alt
                    {
                        exit(0);
                    }
                    auto fi = lockArgs.frameIndex;
                    double deltapc = lockFps(lockArgs);

                    std::cout << "FPS: " << (int)(1000 * perfFreq.QuadPart / deltapc) << " (lock: " << lockFPS << ")" << std::endl;

                    TransformMixer3D tm;
                    tm.rotate(0, 0, 128);
                    pm.render();
                    output->wipe();
                    pm.camara.apply(tm.getTransformMatrix());
                    pm.camara.lookAt({ 0, 0, 0 });
                }
            }

            ~TextureMain()
            {
                delete output;
                delete texture;

                for (auto v : std::set<TextureVertex3DIn*>(vs.begin(), vs.end()))
                {
                    delete v;
                }
            }
        };


    }
}