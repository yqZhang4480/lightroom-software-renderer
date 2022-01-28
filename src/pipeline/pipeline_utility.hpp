#pragma once
#ifndef _SHADING_UTILITY_
#define _SHADING_UTILITY_

#include "../lrutility.hpp"
#include "../drawing.hpp"

namespace lightroom
{

    struct Camara
    {
        Homogeneous<4> position;
        Homogeneous<4> gazeDirection;
        Homogeneous<4> topDirection;
        Angle fov;
        Float f = -10000;

        Camara(const Vector<3>& _position, const Vector<3>& _gazeDirection,
               const Vector<3>& _topDirection, Angle _fov) :
            position(_position), gazeDirection(_gazeDirection), topDirection(_topDirection), fov(_fov) {}

        void lookAt(const Vector<3>& _lookAtPosition)
        {
            gazeDirection.toOrdinary();
            auto& _g = gazeDirection;
            auto _t = topDirection.toOrdinary();
            _g = Homogeneous<4>(Vector<3>(_lookAtPosition - position.toOrdinary()));
            topDirection[0] = ((_g[1] * _g[1] + _g[2] * _g[2]) * _t[0] - _g[0] * (_g[1] * _t[1] + _g[2] * _t[2])) /
                (_g[0] * _g[0] + _g[1] * _g[1] + _g[2] * _g[2]);
            topDirection[1] = (topDirection[0] - _t[0]) * _g[1] / _g[0] + _t[1];
            topDirection[2] = (topDirection[0] - _t[0]) * _g[2] / _g[0] + _t[2];
        }
        void apply(const Matrix<4>& _matrix)
        {
            position.apply(_matrix);
            gazeDirection.apply(_matrix);
            topDirection.apply(_matrix);
        }
        Float getNPlain() const
        {
            return -cos(fov / 2) / sin(fov / 2);
        }
    };

    template <
        std::derived_from<Vertex3DIn> _VertexInType = Vertex3DIn,
        std::derived_from<Vertex3DOut> _VertexOutType = Vertex3DOut,
        std::derived_from<Line3D> _LineType = Line3D,
        std::derived_from<Triangle3D> _TriangleType = Triangle3D>
    class Pipeline final
    {
    public:
        Camara camara;
        Viewport viewport;

    private:
        using VertexInContainer = std::vector<_VertexInType*>;
        using VertexOutContainer = std::vector<_VertexOutType*>;

        VertexOutContainer _vertices;
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

            _deleteAndClearInternalData();
        }

        void clear()
        {
            _deleteAndClearInternalData();
            _deleteAndClearVertices();
        }

        void input(
                PrimitiveInputType _objType, const VertexInContainer& _vertexIns)
        {
            for (auto _v : _vertexIns)
            {
                _vertices.push_back(new _VertexOutType(_v, _objType));
            }
            _vertices.push_back(new _VertexOutType(_vertexIns[0], PrimitiveInputType::NONE));
        }

    private:
        void _deleteAndClearInternalData()
        {
            _depthBuffer.clear();

            for (auto _go : _primitives)
            {
                delete _go;
            }
            _primitives.clear();
        }
        void _deleteAndClearVertices()
        {
            // ASSERT _v1, _v2 in _vertices, _v1 != _v2;
            for (auto _v : _vertices)
            {
                delete _v;
            }
            _vertices.clear();
        }

        inline void _verticesPostProcess()
        {
            for (auto _v : _vertices)
            {
                _v->afterAssemble();
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
                if ((*_last)->primitiveType != (*_first)->primitiveType)
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
            VertexOutContainer::iterator _begin, VertexOutContainer::iterator _end)
        {
            switch ((*_begin)->primitiveType)
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
            VertexOutContainer::iterator _begin, VertexOutContainer::iterator _end)
        {
            for (auto _i = _begin, _j = ++_begin; _i != _end && _j != _end; ++++_i, ++++_j)
            {
                _primitives.push_back(new _LineType({ *_i, *_j }));
            }
        }
        inline void _assembleLineStrip(
            VertexOutContainer::iterator _begin, VertexOutContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            if (_i == _end || _j == _end)
            {
                return;
            }
            for (_i, _j; _j != _end; ++_i, ++_j)
            {
                _primitives.push_back(new _LineType({ *_i, *_j }));
            }
        }
        inline void _assembleLineLoop(
            VertexOutContainer::iterator _begin, VertexOutContainer::iterator _end)
        {
            auto _i = _begin;
            auto _j = ++_begin;
            if (_i == _end || _j == _end)
            {
                return;
            }
            for (_i, _j; _j != _end; ++_i, ++_j)
            {
                _primitives.push_back(new _LineType({ *_i, *_j }));
            }
            _primitives.push_back(new _LineType({ *_i, *_begin }));
        }
        inline void _assembleTriangleStrip(
            VertexOutContainer::iterator _begin, VertexOutContainer::iterator _end)
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
                _primitives.push_back(new _TriangleType({ *_i, *_j, *_k }));
            }
        }
        inline void _assembleTriangleFan(
            VertexOutContainer::iterator _begin, VertexOutContainer::iterator _end)
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
                _primitives.push_back(new _TriangleType({ *_i, *_j, *_k }));
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
                camara.position.toOrdinary(),
                camara.topDirection.toOrdinary().cross(-camara.gazeDirection.toOrdinary()),
                camara.topDirection.toOrdinary(),
                -camara.gazeDirection.toOrdinary())
                .apply(_perspective)
                .apply(_ortho);

            for (auto _v : _vertices)
            {
                _v->apply(_tm.getTransformMatrix());
            }
        }
        void _perspectiveDivision()
        {
            for (auto _v : _vertices)
            {
                _v->position.divide();
            }
        }
        void _viewportTransform()
        {
            TransformMixer3D _tm;
            _tm.scale(viewport.getWidth() / 2.0, -viewport.getHeight() / 2.0, 1)
                .translate((viewport.getWidth() - 1.0) / 2, (viewport.getHeight() - 1.0) / 2, 0);

            for (auto _v : _vertices)
            {
                _v->apply(_tm.getTransformMatrix());
            }
        }
    };
};

#endif // !_SHADING_UTILITY_
