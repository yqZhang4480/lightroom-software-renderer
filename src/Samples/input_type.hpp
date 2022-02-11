#pragma once
#include <iostream>
#include <set>
#include "../lightroom.hpp"
using namespace std;

namespace lightroom
{
    namespace sample
    {
        class IndexedVertex3D : public Vertex3D
        {
        public:
            inline static size_t INDEX = 0;

            size_t index;
            vector<Triangle3D<IndexedVertex3D>*> triangles;
            vector<Line3D<IndexedVertex3D>*> lines;
            IndexedVertex3D(const Vertex3DIn* vin, PrimitiveInputType p) :
                Vertex3D(vin, p), index(INDEX++){}

            inline void whenRegisteredByTriangle(Triangle3D<IndexedVertex3D>* t)
            {
                triangles.push_back(t);
            }
            inline void whenRegisteredByLine(Line3D<IndexedVertex3D>* t)
            {
                lines.push_back(t);
            }
            inline void afterAssemble()
            {
                cout << "Vertex " << index << " is registered by" << endl;
                for (auto& l : lines)
                {
                    cout << "line " << (const void*)l << endl;
                }
                for (auto& t : triangles)
                {
                    cout << "triangle " << (const void*)t << endl;
                }
            }
        };

        class InputType
        {
        private:
            SequenceMap* output;
            std::vector<Vertex3DIn*> vs;
        public:
            InputType() :
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

                auto pm = Pipeline<IndexedVertex3D, Line3D<IndexedVertex3D>, Triangle3D<IndexedVertex3D>>(camara, output);

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

            ~InputType()
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