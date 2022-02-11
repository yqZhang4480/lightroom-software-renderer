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

        // ”¶”√±‰ªªæÿ’Û
        inline void apply(const TransformMixer3D& _mixer);

    protected:
        inline ~Vertex3D();
        template <
            typename _T1,
            typename _T2,
            typename _T3> requires
            std::is_convertible_v<const _T1*, const  Vertex3D*> &&
            std::is_convertible_v<const _T2*, const  Line3D<_T1>*> &&
            std::is_convertible_v<const _T3*, const  Triangle3D<_T1>*>
             friend class Pipeline;
        template <typename _T> requires std::is_convertible_v<const _T*, const  Vertex3D*> 
        friend class Triangle3D;
        template <typename _T> requires std::is_convertible_v<const _T*, const  Vertex3D*> 
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
    inline Vertex3D::~Vertex3D() {}
    inline void Vertex3D::apply(const TransformMixer3D& _mixer)
    {
        position.apply(_mixer);
    }
    template <typename _VertexType>
    inline void Vertex3D::whenRegisteredByTriangle(Triangle3D<_VertexType>*) {}
    template <typename _VertexType>
    inline void Vertex3D::whenRegisteredByLine(Line3D<_VertexType>*) {}
    inline void Vertex3D::afterAssemble() {}
};