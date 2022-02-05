#pragma once
#include <iostream>
#include <set>
#include "../lightroom.hpp"

namespace lightroom
{
    namespace sample
    {
        class InputTypeMain
        {
        private:
            SequenceMap* output;
            std::vector<Vertex3DIn*> vs;
        public:
            InputTypeMain() :
                vs({
                    new Vertex3DIn(Vector<3>(-15, -10, 0)),
                    new Vertex3DIn(Vector<3>(-5,  10, 0)),
                    new Vertex3DIn(Vector<3>(5, -10, 0)),
                    new Vertex3DIn(Vector<3>(15,  10, 0)) })
            {
                SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
                int w = GetSystemMetrics(SM_CXSCREEN);
                int h = GetSystemMetrics(SM_CYSCREEN);
                output = new SequenceMap(PxCoordinate{ w, h });

                auto camara = Camara(Vector<3>(0, 0, 50), Vector<3>(0, 0, -1), Vector<3>(0, 1, 0), 1.36);

                auto pm = Pipeline<Vertex3D, Line3D<Vertex3D>, Triangle3D<Vertex3D>>(camara, output);

                pm.input(PrimitiveInputType::LINES, vs);
                pm.render();
                getchar();
                pm.clear();
                output->wipe();

                pm.input(PrimitiveInputType::LINE_STRIP, vs);
                pm.render();
                getchar();
                pm.clear();
                output->wipe();

                pm.input(PrimitiveInputType::LINE_LOOP, vs);
                pm.render();
                getchar();
                pm.clear();
                output->wipe();

                pm.input(PrimitiveInputType::TRIANGLE_STRIP, vs);
                pm.render();
                getchar();
                pm.clear();
                output->wipe();

                pm.input(PrimitiveInputType::TRIANGLE_FAN, vs);
                pm.render();
                getchar();
                output->wipe();
            }

            ~InputTypeMain()
            {
                delete output;

                for (auto v : std::set<Vertex3DIn*>(vs.begin(), vs.end()))
                {
                    delete v;
                }
            }
        };
    }
}