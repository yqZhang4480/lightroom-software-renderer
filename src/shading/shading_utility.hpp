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
        PointLight(const Vector<3>& position, const Color& color) :
            position(position), color(color) {}

        Homogeneous<4> position;
        Color color;
    };

    struct Camara
    {
        Homogeneous<4> position;
        Vector<3> gazeDirection;
        Vector<3> topDirection;
        Angle fov;

        Camara(const Vector<3>& _position, const Vector<3>& _gazeDirection,
               const Vector<3>& _topDirection, Angle _fov) :
            position(_position), gazeDirection(_gazeDirection), topDirection(_topDirection), fov(_fov) {}
    };

    class PipelineManager
    {
        using Vertex3DInContainer = std::list<Vertex3DIn>;
        using Vertex3DOutContainer = std::vector<Vertex3DOut>;
    public:
        PipelineManager() :
            _mask(_viewport.getWidth()* _viewport.getHeight(), false),
            _refBuffer(_viewport.getWidth() * _viewport.getHeight(), nullptr),
            _depthBuffer(_viewport.getWidth() * _viewport.getHeight(), -1)
        {
            _colorMap = new SequenceMap(PxCoordinate{ _viewport.getWidth(), _viewport.getHeight() });
        }

        void useCamara(size_t _camaraIndex)
        {
            this->_camaraIndex = _camaraIndex;
        }
        void clear()
        {
            _clearUserData();
            _clearInternalData();
        }

        void render()
        {
            _prepare();
            _assemble();
            _evaluateVertexsNormal();
            _vertexsShade();
            _mvpTransform();
            _perspectiveDivision();
            _viewportTransform();


            // TODO
            auto& _camara = _camaras[_camaraIndex];

            constexpr const Float _r = 1;
            constexpr const Float _t = 1;
            constexpr const Float _f = -10000;
            Float _n = -_r * cos(_camara.fov / 2) / sin(_camara.fov / 2);
            for (auto _graphObj : _graphObjects)
            {

                _graphObj->draw(_colorMap, _mask, _depthBuffer, _refBuffer, _n, _f);
            }
            _viewport.print(_colorMap, _mask);
        }

        std::pair<Vertex3DInContainer::iterator, Vertex3DInContainer::iterator>
            addGraphObject(
                PrimitiveType _objType,
                const std::vector<std::pair<Vector<3>, PxCoordinate>>& _positionTexturePairs,
                ColorMap* _texture)
        {
            auto _begin = _vertexIns.end();
            for (auto& _p : _positionTexturePairs)
            {
                _vertexIns.emplace_back(_objType, _p.first, _p.second, _texture);
            }
            auto _end = _vertexIns.end();
            _vertexIns.emplace_back(PrimitiveType::NONE, Homogeneous<4>(Vector<3>{ 0,0,0 }), PxCoordinate(0,0), nullptr);
            return { _begin, _end };
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
        Vertex3DInContainer _vertexIns;
        Vertex3DOutContainer _vertexOuts;
        std::vector<GraphObj3D*> _graphObjects;
        std::vector<PointLight> _pointLights;

        Viewport _viewport;
        std::vector<Camara> _camaras;
        Color _ambient;

        WritableColorMap* _colorMap;
        OverwriteMask _mask;
        DepthBuffer _depthBuffer;
        ReferenceBuffer _refBuffer;

        size_t _camaraIndex;

        inline void _prepare()
        {
            _clearInternalData();

            for (const auto& _v : _vertexIns)
            {
                _vertexOuts.emplace_back(_v.primitiveType, _v.position, _v.texturePosition, _v.texture);
            }
        }

        void _clearUserData()
        {
            _vertexIns.clear();
            _camaras.clear();
            _pointLights.clear();
        }
        void _clearInternalData()
        {
            _mask.clear();
            _refBuffer.clear();
            _depthBuffer.clear();
            _vertexOuts.clear();
            for (auto _go : _graphObjects)
            {
                delete _go;
            }
            _graphObjects.clear();
        }

        void _assemble()
        {
            auto _end = _vertexOuts.end();
            auto _first = _vertexOuts.begin();
            for (auto _last = _vertexOuts.begin(); ++_last != _end;)
            {
                if (_last->primitiveType != _first->primitiveType)
                {
                    _assembleDeliver(_first, _last);
                    _first = ++_last;
                    if (_first == _end)
                    {
                        break;
                    }
                }
            }
        }
        void _assembleDeliver(
            Vertex3DOutContainer::iterator _begin, Vertex3DOutContainer::iterator _end)
        {
            switch (_begin->primitiveType)
            {
                case lightroom::PrimitiveType::LINES:
                    _assembleLines(_begin, _end);
                    break;
                case lightroom::PrimitiveType::LINE_STRIP:
                    _assembleLineStrip(_begin, _end);
                    break;
                case lightroom::PrimitiveType::LINE_LOOP:
                    _assembleLineLoop(_begin, _end);
                    break;
                case lightroom::PrimitiveType::TRIANGLE_STRIP:
                    _assembleTriangleStrip(_begin, _end);
                    break;
                case lightroom::PrimitiveType::TRIANGLE_FAN:
                    _assembleTriangleFan(_begin, _end);
                    break;
                default:
                    break;
            }
        }
        inline void _assembleLines(
            Vertex3DOutContainer::iterator _begin, Vertex3DOutContainer::iterator _end)
        {
            for (auto _i = _begin, _j = ++_begin; _i != _end && _j != _end; ++++_i, ++++_j)
            {
                _graphObjects.push_back(new Line3D({ &*_i, &*_j }));
            }
        }
        inline void _assembleLineStrip(
            Vertex3DOutContainer::iterator _begin, Vertex3DOutContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            if (_i == _end || _j == _end)
            {
                return;
            }
            for (_i, _j; _j != _end; ++_i, ++_j)
            {
                _graphObjects.push_back(new Line3D({ &*_i, &*_j }));
            }
        }
        inline void _assembleLineLoop(
            Vertex3DOutContainer::iterator _begin, Vertex3DOutContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            if (_i == _end || _j == _end)
            {
                return;
            }
            for (_i, _j; _j != _end; ++_i, ++_j)
            {
                _graphObjects.push_back(new Line3D({ &*_i, &*_j }));
            }
            _graphObjects.push_back(new Line3D({ &*_i, &*_begin }));
        }
        inline void _assembleTriangleStrip(
            Vertex3DOutContainer::iterator _begin, Vertex3DOutContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            auto _k = ++_begin;
            if (_i == _end || _j == _end || _k == _end)
            {
                return;
            }
            
            for (_i, _j, _k; _k != _end; ++_i, ++_j, ++_k)
            {
                _graphObjects.push_back(new Triangle3D({ &*_i, &*_j, &*_k }));
            }
        }
        inline void _assembleTriangleFan(
            Vertex3DOutContainer::iterator _begin, Vertex3DOutContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            auto _k = ++_begin;
            if (_i == _end || _j == _end || _k == _end)
            {
                return;
            }
            for (_j, _k; _k != _end; ++_j, ++_k)
            {
                _graphObjects.push_back(new Triangle3D({ &*_i, &*_j, &*_k }));
            }
        }

        void _evaluateVertexsNormal()
        {
            for (auto& _v : _vertexOuts)
            {
                Float _sumArea = 0;
                Vector<3> _normal{ 0,0,0 };
                for (const auto _t : _v.relatedTriangles)
                {
                    auto _area = _t->evaluateAreaSquare();
                    _sumArea += _area;
                    _normal += _area * _t->evaluateNormal();
                }
                _v.normal = _normal.normalized();
            }
        }

        void _vertexsShade()
        {
            _vertexAmbientShade();
            _vertexDiffuseShade();
        }
        void _vertexAmbientShade()
        {
            for (auto& _p : _vertexOuts)
            {
                _p.color = _ambient;
            }
        }
        void _vertexDiffuseShade()
        {
            for (auto& _pl : _pointLights)
            {
                for (auto& _v : _vertexOuts)
                {
                    Vector<3> _l =
                        _pl.position.toOrdinary() - _v.position.toOrdinary();
                    Float _diffuse = std::max<Float>(Float(0), -_v.normal.dot(_l.normalized()));
                    _v.color += _pl.color * _diffuse;
                }
            }
        }

        void _mvpTransform()
        {
            TransformMixer3D _tm;
            auto& _camara = _camaras[_camaraIndex];

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
                _camara.position.toOrdinary(),
                _camara.topDirection.cross(-_camara.gazeDirection),
                _camara.topDirection,
                -_camara.gazeDirection)
                .apply(_perspective)
                .apply(_ortho);

            for (auto& _v : _vertexOuts)
            {
                _v.apply(_tm.getTransformMatrix());
            }
        }
        void _perspectiveDivision()
        {
            for (auto& _v : _vertexOuts)
            {
                _v.position.divide();
            }
        }
        void _viewportTransform()
        {
            TransformMixer3D _tm;
            _tm.scale(_viewport.getWidth() / 2.0, -_viewport.getHeight() / 2.0, 1)
                .translate((_viewport.getWidth() - 1.0) / 2, (_viewport.getHeight() - 1.0) / 2, 0);

            for (auto& _v : _vertexOuts)
            {
                _v.apply(_tm.getTransformMatrix());
            }
        }
    };
};

#endif // !_SHADING_UTILITY_
