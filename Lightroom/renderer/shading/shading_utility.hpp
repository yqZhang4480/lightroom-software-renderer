#pragma once
#ifndef _SHADING_UTILITY_
#define _SHADING_UTILITY_

#include "../lrutility.hpp"
#include "../drawing.hpp"

namespace lightroom
{
    class PointLight : public Vertex3D
    {
    public:
        PointLight(const Homogeneous<4>& _position, const NormalizedColor& _intensity) :
            Vertex3D(_position, Homogeneous<4>({ 0.0, 0.0, 0.0 }), _intensity) {}
        ~PointLight() {}
    };

    struct Camara
    {
        Homogeneous<4> position;
        Homogeneous<4> gazeDirection;
        Homogeneous<4> topDirection;
        Angle fov;
    };

    class SceneManager
    {
    public:
        SceneManager& useCamara(size_t _camaraIndex)
        {
            this->_camaraIndex = _camaraIndex;
        }

        void render()
        {
            _shade();
            for (auto& _graphObj : _graphObjects)
            {
                _graphObj.draw(_ncm, _depthBuffer, _refBuffer);
            }
        }


    private:
        std::list<Vertex3D> _vertexs;
        std::list<GraphObj3D> _graphObjects;

        std::vector<Camara> _camaras;
        std::vector<PointLight> _pointLights;
        NormalizedColor _ambient;

        NormalizedColorMap _ncm;
        DepthBuffer _depthBuffer;
        ReferenceBuffer _refBuffer;

        size_t _camaraIndex;

        void _shade()
        {

        }
    };
};

#endif // !_SHADING_UTILITY_
