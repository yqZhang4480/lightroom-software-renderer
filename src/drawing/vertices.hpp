#pragma once
#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class Vertex3DIn
    {
    public:
        Homogeneous position;
        Vertex3DIn(const Vector<3>& position) : position(position) {}
    };
    class Vertex3DOut
    {
    public:
        PrimitiveInputType primitiveType;
        Homogeneous position;
        const Vertex3DIn* vertexInRef;

        Vertex3DOut(const Vertex3DIn* _vin,
                    PrimitiveInputType primitiveType) :
            primitiveType(primitiveType), position(_vin->position), vertexInRef(_vin) {}
        virtual ~Vertex3DOut() {}

        Vertex3DOut& apply(const Matrix<4>& _transformMatrix)
        {
            position.apply(_transformMatrix);
            return *this;
        }
    protected:
         template <
            std::derived_from<Vertex3DIn>,
            std::derived_from<Vertex3DOut>,
            std::derived_from<Line3D>,
            std::derived_from<Triangle3D>>
             friend class Pipeline;
        friend class Triangle3D;
        friend class Line3D;

        virtual void whenRegisteredByTriangle(Triangle3D*) {}
        virtual void whenRegisteredByLine(Line3D*) {}
        virtual void afterAssemble() {}
    };

   
};