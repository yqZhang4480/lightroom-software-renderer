#pragma once
#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class Vertex3DIn
    {
    public:
        Homogeneous<4> position;
        Vertex3DIn(const Vector<3>& position) : position(position) {}
    };
    class Vertex3DOut
    {
    public:
        PrimitiveInputType primitiveType;
        Homogeneous<4> position;
        const Vertex3DIn* vertexInRef;

        Vertex3DOut(const Vertex3DIn* _vin,
                    PrimitiveInputType primitiveType) :
            primitiveType(primitiveType), position(_vin->position), vertexInRef(_vin) {}

        Vertex3DOut& apply(const Matrix<4>& _transformMatrix)
        {
            position.apply(_transformMatrix);
            return *this;
        }
    protected:
        friend class Triangle3D;
        friend class Line3D;

        virtual void whenRegisteredByTriangle(Triangle3D*) {}
        virtual void whenRegisteredByLine(Line3D*) {}
    };

    class TextureVertex3DIn : public Vertex3DIn
    {
    public:
        PxCoordinate texturePosition;
        ColorMap* texture;

        TextureVertex3DIn(const Vector<3>& position,
                          const PxCoordinate& texturePosition, ColorMap* texture) :
            Vertex3DIn(position), texturePosition(texturePosition), texture(texture) {}
    };
    class TextureVertex3DOut : public Vertex3DOut
    {
    public:
        PxCoordinate texturePosition;
        ColorMap* texture;

        TextureVertex3DOut(const TextureVertex3DIn* _vin,
                           PrimitiveInputType primitiveType) :
            Vertex3DOut(_vin, primitiveType),
            texturePosition(_vin->texturePosition), texture(_vin->texture) {}
    };
};