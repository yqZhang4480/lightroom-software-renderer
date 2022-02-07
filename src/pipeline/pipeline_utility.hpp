#pragma once
#ifndef _PIPELINE_UTILITY_
#define _PIPELINE_UTILITY_

#include "../lrutility.hpp"
#include "../drawing.hpp"

namespace lightroom
{

    struct Camara
    {
        Homogeneous position;
        Homogeneous gazeDirection;
        Homogeneous topDirection;
        Angle fov;
        Float f = -10000;

        inline Camara(const Vector<3>& _position, const Vector<3>& _gazeDirection,
               const Vector<3>& _topDirection, Angle _fov);


        /*
        inline void lookAt(const Vector<3>& _lookAtPosition)
        {
            gazeDirection.toCartesian();
            auto& _g = gazeDirection;
            auto _t = topDirection.toCartesian();
            _g = Homogeneous(Vector<3>(_lookAtPosition - position.toCartesian()));
            topDirection[0] = ((_g[1] * _g[1] + _g[2] * _g[2]) * _t[0] - _g[0] * (_g[1] * _t[1] + _g[2] * _t[2])) /
                (_g[0] * _g[0] + _g[1] * _g[1] + _g[2] * _g[2]);
            topDirection[1] = (topDirection[0] - _t[0]) * _g[1] / _g[0] + _t[1];
            topDirection[2] = (topDirection[0] - _t[0]) * _g[2] / _g[0] + _t[2];
        }
        */

        inline void apply(const TransformMixer3D& _mixer, bool _keepDirection = false);
        inline Float getNPlain() const;
    };

    inline Camara::Camara(const Vector<3>& _position, const Vector<3>& _gazeDirection,
                   const Vector<3>& _topDirection, Angle _fov) :
        position(_position), gazeDirection(_gazeDirection, 0), topDirection(_topDirection, 0), fov(_fov) {}
    inline void Camara::apply(const TransformMixer3D& _mixer, bool _keepDirection)
    {
        position.apply(_mixer);
        if (!_keepDirection)
        {
            gazeDirection.apply(_mixer);
            topDirection.apply(_mixer);
        }
    }
    inline Float Camara::getNPlain() const
    {
        return -cos(fov / 2) / sin(fov / 2);
    }


    template <
        std::derived_from<Vertex3D> _VertexType,
        std::derived_from<Line3D<_VertexType>> _LineType,
        std::derived_from<Triangle3D<_VertexType>> _TriangleType>
    class Pipeline final
    {
    public:
        Camara camara;
        Viewport viewport;

    private:
        using VertexContainer = std::vector<_VertexType>;

        VertexContainer _vertices;
        std::vector<GraphObj3D*> _primitives;
        DepthBuffer _depthBuffer;

    public:
        Pipeline(const Camara& camara,
                        WritableColorMap* output) :
            _depthBuffer(viewport.getWidth() * viewport.getHeight(), -1),
            camara(camara),
            viewport(output) {}
        ~Pipeline()
        {
            clear();
        }

        void render()
        {
            _mvpTransform();
            _perspectiveDivision();
            _viewportTransform();
            _assemble();
            _verticesPostProcess();

            for (auto _primitive : _primitives)
            {
                _primitive->draw(viewport.output, _depthBuffer, camara.getNPlain(), camara.f);
            }
            viewport.print();

            _clearInternalData();
        }

        void clear()
        {
            _clearInternalData();
            _clearVertices();
        }

        template <std::derived_from<Vertex3DIn> _VertexInType>
        inline void input(PrimitiveInputType _inputType, const std::vector<_VertexInType*>& _vertexIns)
        {
            for (auto& _v : _vertexIns)
            {
                _vertices.emplace_back(_v, _inputType);
            }
            _vertices.emplace_back(_vertexIns[0], PrimitiveInputType::NONE);
        }

    private:
        void _clearInternalData()
        {
            _depthBuffer.clear();

            for (auto _go : _primitives)
            {
                delete _go;
            }
            _primitives.clear();
        }
        void _clearVertices()
        {
            _vertices.clear();
        }

        inline void _verticesPostProcess()
        {
            for (auto& _v : _vertices)
            {
                _v.afterAssemble();
            }
        }

        void _assemble()
        {
            auto _end = _vertices.end();
            auto _first = _vertices.begin();
            if (_end == _first)
            {
                return;
            }
            for (auto _last = _vertices.begin(); ++_last != _end;)
            {
                if ((*_last).primitiveType != (*_first).primitiveType)
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
            VertexContainer::iterator _begin, VertexContainer::iterator _end)
        {
            switch ((*_begin).primitiveType)
            {
                case lightroom::PrimitiveInputType::LINES:
                    _assembleLines(_begin, _end);
                    break;
                case lightroom::PrimitiveInputType::LINE_STRIP:
                    _assembleLineStrip(_begin, _end);
                    break;
                case lightroom::PrimitiveInputType::LINE_LOOP:
                    _assembleLineLoop(_begin, _end);
                    break;
                case lightroom::PrimitiveInputType::TRIANGLE_STRIP:
                    _assembleTriangleStrip(_begin, _end);
                    break;
                case lightroom::PrimitiveInputType::TRIANGLE_FAN:
                    _assembleTriangleFan(_begin, _end);
                    break;
                default:
                    break;
            }
        }
        inline void _assembleLines(
            VertexContainer::iterator _begin, VertexContainer::iterator _end)
        {
            for (auto _i = _begin, _j = ++_begin; _i != _end && _j != _end; ++++_i, ++++_j)
            {
                _primitives.push_back(new _LineType({ &*_i, &*_j }));
            }
        }
        inline void _assembleLineStrip(
            VertexContainer::iterator _begin, VertexContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            if (_i == _end || _j == _end)
            {
                return;
            }
            for (_i, _j; _j != _end; ++_i, ++_j)
            {
                _primitives.push_back(new _LineType({ &*_i, &*_j }));
            }
        }
        inline void _assembleLineLoop(
            VertexContainer::iterator _begin, VertexContainer::iterator _end)
        {
            auto __begin = _begin;
            auto _i = __begin;
            auto _j = ++__begin;
            if (_i == _end || _j == _end)
            {
                return;
            }
            for (_i, _j; _j != _end; ++_i, ++_j)
            {
                _primitives.push_back(new _LineType({ &*_i, &*_j }));
            }
            _primitives.push_back(new _LineType({ &*_i, &*_begin }));
        }
        inline void _assembleTriangleStrip(
            VertexContainer::iterator _begin, VertexContainer::iterator _end)
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
                _primitives.push_back(new _TriangleType({ &*_i, &*_j, &*_k }));
            }
        }
        inline void _assembleTriangleFan(
            VertexContainer::iterator _begin, VertexContainer::iterator _end)
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
                _primitives.push_back(new _TriangleType({ &*_i, &*_j, &*_k }));
            }
        }

        void _mvpTransform()
        {
            TransformMixer3D _tm;

            Float _t = static_cast<lightroom::Float>(viewport.getHeight()) / viewport.getWidth();
            Float _f = camara.f;
            Float _n = camara.getNPlain();
            Matrix<4> _perspective, _ortho;
            _perspective <<
                _n, 0, 0, 0,
                0, _n, 0, 0,
                0, 0, _n + _f, -_n * _f,
                0, 0, 1, 0;
            _ortho <<
                1, 0, 0, 0,
                0, Float(1) / _t, 0, 0,
                0, 0, Float(2) / (_n - _f), -(_n + _f) / (_n - _f),
                0, 0, 0, 1;

            _tm.changeBase(
                camara.position.toCartesian(),
                camara.topDirection.toCartesian().cross(-camara.gazeDirection.toCartesian()),
                camara.topDirection.toCartesian(),
                -camara.gazeDirection.toCartesian())
                .apply(_perspective)
                .apply(_ortho);

            for (auto& _v : _vertices)
            {
                _v.apply(_tm);
            }
        }
        void _perspectiveDivision()
        {
            for (auto& _v : _vertices)
            {
                _v.position.divide();
            }
        }
        void _viewportTransform()
        {
            TransformMixer3D _tm;
            _tm.scale(viewport.getWidth() / 2.0, -viewport.getHeight() / 2.0, 1)
                .translate((viewport.getWidth() - 1.0) / 2, (viewport.getHeight() - 1.0) / 2, 0);

            for (auto& _v : _vertices)
            {
                _v.apply(_tm);
            }
        }
    };
};

#endif // !_PIPELINE_UTILITY_
