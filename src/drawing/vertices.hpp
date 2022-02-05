#pragma once
#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class Vertex3DIn
    {
    public:
        Homogeneous position;

        inline Vertex3DIn(const Vector<3>& position);
    };
    class Vertex3D
    {
    public:
        PrimitiveInputType primitiveType;
        Homogeneous position;
        const Vertex3DIn* vertexInRef;

        inline Vertex3D(const Vertex3DIn* _vin, PrimitiveInputType primitiveType);

        // Ӧ�ñ任����
        inline void apply(const TransformMixer3D& _transformMatrix);

    protected:
        inline ~Vertex3D();
        template <
            std::derived_from<Vertex3DIn>,
            std::derived_from<Vertex3D> _VertexType,
            std::derived_from<Line3D<_VertexType>>,
            std::derived_from<Triangle3D<_VertexType>>>
             friend class Pipeline;
        template <std::derived_from<Vertex3D>>
        friend class Triangle3D;
        template <std::derived_from<Vertex3D>>
        friend class Line3D;

        template <typename _VertexType>
        inline void whenRegisteredByTriangle(Triangle3D<_VertexType>*);
        template <typename _VertexType>
        inline void whenRegisteredByLine(Line3D<_VertexType>*);
        inline void afterAssemble();
    };

    inline Vertex3DIn::Vertex3DIn(const Vector<3>& position) : position(position) {}

    inline Vertex3D::Vertex3D(const Vertex3DIn* _vin,
             PrimitiveInputType primitiveType) :
        primitiveType(primitiveType), position(_vin->position), vertexInRef(_vin) {}
    Vertex3D::~Vertex3D() {}
    inline void Vertex3D::apply(const TransformMixer3D& _transformMatrix)
    {
        position.apply(_transformMatrix);
    }
    template <typename _VertexType>
    void Vertex3D::whenRegisteredByTriangle(Triangle3D<_VertexType>*) {}
    template <typename _VertexType>
    void Vertex3D::whenRegisteredByLine(Line3D<_VertexType>*) {}
    void Vertex3D::afterAssemble() {}
};