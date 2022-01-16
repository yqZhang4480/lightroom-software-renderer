#pragma once
#ifndef _SHADING_UTILITY_
#define _SHADING_UTILITY_

#include "../lrutility.hpp"
#include "../drawing.hpp"

namespace lightroom
{
    class PointLight
    {
    public:
        PointLight(Vertex3D* _vertex) : vertex(_vertex) {}

        Vertex3D* vertex;
    };

    struct Camara
    {
        Vertex3D* position;
        Vector<3> gazeDirection;
        Vector<3> topDirection;
        Angle fov;

        Camara(Vertex3D* _position, const Vector<3>& _gazeDirection, const Vector<3>& _topDirection, Angle _fov) :
            position(_position), gazeDirection(_gazeDirection), topDirection(_topDirection), fov(_fov) {}
    };

    class PipelineManager
    {
    public:
        PipelineManager() :
            _ncm({ _viewport.getWidth(), _viewport.getHeight() }),
            _mask(_viewport.getWidth()* _viewport.getHeight(), false),
            _refBuffer(_viewport.getWidth() * _viewport.getHeight(), nullptr),
            _depthBuffer(_viewport.getWidth() * _viewport.getHeight(), -1)
        {}
        ~PipelineManager()
        {
            for (auto& _go : _graphObjects)
            {
                delete _go;
            }
        }

        void useCamara(size_t _camaraIndex)
        {
            this->_camaraIndex = _camaraIndex;
        }

        void render()
        {
            _prepare();
            _vertexsShade();
            _mvpTransform();
            _perspectiveDivision();
            _viewportTransform();
            for (auto _graphObj : _graphObjects)
            {
                _graphObj->draw(_ncm, _mask, _vertexsOut, _depthBuffer, _refBuffer);
            }
            _viewport.print(_ncm, _mask);
        }

        template <typename... _Args>
        Vertex3D* addVertex(_Args&&... _args)
        {
            _vertexsIn.emplace_front(_args...);

            auto _ret = &_vertexsIn.front();
            _vertexsOut.insert({ _ret, *_ret });
            return _ret;
        }
        template <GraphObjType _GRAPH_OBJ_TYPE, typename... _Args>
        GraphObj3D* addGraphObject(_Args&&... _args)
        {
            if constexpr (_GRAPH_OBJ_TYPE == lightroom::GraphObjType::LINE)
            {
                _graphObjects.push_back(new Line3D(_args...));
            }
            else if (_GRAPH_OBJ_TYPE == lightroom::GraphObjType::TRIANGLE)
            {
                _graphObjects.push_back(new Triangle3D(_args...));
            }
            return _graphObjects.back();
        }
        template <typename... _Args>
        void addPointLight(_Args&&... _args)
        {
            _pointLights.emplace_back(_args...);
        }
        template <typename... _Args>
        size_t addCamara(_Args&&... _args)
        {
            _camaras.emplace_back(_args...);
            return _camaras.size() - 1;
        }

    public:
        std::list<Vertex3D> _vertexsIn;
        std::unordered_map<const Vertex3D*, Vertex3D> _vertexsOut;
        std::list<GraphObj3D*> _graphObjects;
        std::vector<PointLight> _pointLights;

        Viewport _viewport;
        std::vector<Camara> _camaras;
        NormalizedColor _ambient;

        NormalizedColorMap _ncm;
        OverwriteMask _mask;
        DepthBuffer _depthBuffer;
        ReferenceBuffer _refBuffer;

        size_t _camaraIndex;

        inline void _vertexsShade()
        {
            _vertexAmbientShade();
            _vertexDiffuseShade();
        }
        inline void _prepare()
        {
            _mask       .clear();
            _refBuffer  .clear();
            _depthBuffer.clear();
            _vertexsOut .clear();

            const auto _end = _vertexsIn.cend();
            for (auto _i = _vertexsIn.cbegin(); _i != _end; _i++)
            {
                _vertexsOut[&*_i] = *_i;
            }
        }
        inline void _vertexAmbientShade()
        {
            for (auto& _p : _vertexsOut)
            {
                _p.second.color = _ambient;
            }
        }
        inline void _vertexDiffuseShade()
        {
            for (const auto& _pl : _pointLights)
            {
                for (auto& _p : _vertexsOut)
                {
                    auto& _v = _p.second;
                    Vector<3> _l = _pl.vertex->position.toOrdinaryCoordinate() - _v.position.toOrdinaryCoordinate();
                    Float _diffuse = std::max<Float>(Float(0), -_v.normal.toOrdinaryCoordinate().dot(_l.normalized()));
                    _v.color += _pl.vertex->color * _diffuse;
                }
            }
        }

        inline void _mvpTransform()
        {
            TransformMixer3D _tm;
            const auto& _camara = _camaras[_camaraIndex];

            constexpr const Float _r = 1;
            constexpr const Float _t = 1;
            constexpr const Float _f = -10000;
            Float _n = -_r * cos(_camara.fov / 2) / sin(_camara.fov / 2);
            Matrix<4> _perspective, _ortho;
            _perspective <<
                _n, 0, 0, 0,
                0, _n, 0, 0,
                0, 0, _n + _f, -_n * _f,
                0, 0, Float(1), 0;
            _ortho <<
                Float(1) / _r, 0, 0, 0,
                0, Float(1) / _t, 0, 0,
                0, 0, Float(2) / (_n - _f), -(_n + _f) / (_n - _f),
                0, 0, 0, Float(1);

            _tm.changeBase(
                _camara.position->position.toOrdinaryCoordinate(),
                _camara.topDirection.cross(-_camara.gazeDirection),
                _camara.topDirection,
                -_camara.gazeDirection)
                .apply(_perspective)
                .apply(_ortho);

            for (auto& _p : _vertexsOut)
            {
                _p.second.apply(_tm.getTransformMatrix());
            }
        }
        inline void _perspectiveDivision()
        {
            for (auto& _p : _vertexsOut)
            {
                _p.second.position.divide();
            }
        }
        inline void _viewportTransform()
        {
            TransformMixer3D _tm;
            _tm.scale(_viewport.getWidth() / 2.0, -_viewport.getHeight() / 2.0, 1)
                .translate((_viewport.getWidth() - 1.0) / 2, (_viewport.getHeight() - 1.0) / 2, 0);

            for (auto& _p : _vertexsOut)
            {
                _p.second.apply(_tm.getTransformMatrix());
            }
        }
    };
};

#endif // !_SHADING_UTILITY_
