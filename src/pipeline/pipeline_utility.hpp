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
        Vector<3> gazeDirection;
        Vector<3> topDirection;
        Angle fov;
        Float f = -10000;

        Camara(const Vector<3>& _position, const Vector<3>& _gazeDirection,
               const Vector<3>& _topDirection, Angle _fov) :
            position(_position), gazeDirection(_gazeDirection), topDirection(_topDirection), fov(_fov) {}

        void lookAt(const Vector<3>& _lookAtPosition)
        {
            auto& _g = gazeDirection;
            auto& _t = topDirection;
            gazeDirection = (_lookAtPosition - position.toOrdinary()).normalized();
            topDirection[0] = (_t[0] * _g[1] - _t[1] * _g[0]) * _g[1] / (_g[0] * _g[0] + _g[1] * _g[1]);
            topDirection[1] = -_g[0] * _t[0] / _g[1];
        }
        void apply(const Matrix<4>& _matrix)
        {
            position.apply(_matrix);
        }
        Float getNPlain() const
        {
            return -cos(fov / 2) / sin(fov / 2);
        }
    };

    class VertexProcessor
    {
    public:
        void operator()(Vertex3DOut* _vout) {}
    };
    class VertexPostProcessor
    {
    public:
        void operator()(Vertex3DOut* _vout) {}
    };

    template <
        std::derived_from<Vertex3DIn> _VertexInType = Vertex3DIn,
        std::derived_from<Vertex3DOut> _VertexOutType = Vertex3DOut,
        std::derived_from<Line3D> _LineType = Line3D,
        std::derived_from<Triangle3D> _TriangleType = Triangle3D,
        typename _VertexProcessor = VertexProcessor,
        typename _VertexPostProcessor = VertexPostProcessor>
    class PipelineManager final
    {
    public:
        Camara camara;
        Viewport viewport;
        _VertexProcessor vertexProcessor;
        _VertexPostProcessor vertexPostProcessor;

    private:
        using VertexInContainer = std::vector<_VertexInType*>;
        using VertexOutContainer = std::vector<_VertexOutType*>;

        VertexOutContainer _vertexOuts;
        std::vector<GraphObj3D*> _graphObjects;
        DepthBuffer _depthBuffer;

    public:
        PipelineManager(const Camara& camara,
                        WritableColorMap* output,
                        const _VertexProcessor& vertexProcessor = {},
                        const _VertexPostProcessor& vertexPostProcessor = {}) :
            _depthBuffer(viewport.getWidth() * viewport.getHeight(), -1),
            camara(camara),
            viewport(output),
            vertexProcessor(vertexProcessor),
            vertexPostProcessor(vertexPostProcessor) {}
        ~PipelineManager()
        {
            clear();
        }

        void render()
        {
            _verticesProcess();
            _mvpTransform();
            _perspectiveDivision();
            _viewportTransform();
            _assemble();
            _verticesPostProcess();

            for (auto _graphObj : _graphObjects)
            {
                _graphObj->draw(viewport.output, _depthBuffer, camara.getNPlain(), camara.f);
            }
            viewport.print();

            _deleteAndClearInternalData();
        }

        void clear()
        {
            _deleteAndClearInternalData();
            _deleteAndClearVertices();
        }

        void addGraphObject(
                PrimitiveInputType _objType, const VertexInContainer& _vertexIns)
        {
            for (auto _v : _vertexIns)
            {
                _vertexOuts.push_back(new _VertexOutType(_v, _objType));
            }
            _vertexOuts.push_back(new _VertexOutType(_vertexIns[0], PrimitiveInputType::NONE));
        }

    private:
        void _deleteAndClearInternalData()
        {
            _depthBuffer.clear();

            for (auto _go : _graphObjects)
            {
                delete _go;
            }
            _graphObjects.clear();
        }
        void _deleteAndClearVertices()
        {
            // ASSERT _v1, _v2 in _vertexOuts, _v1 != _v2;
            for (auto _v : _vertexOuts)
            {
                delete _v;
            }
            _vertexOuts.clear();
        }

        inline void _verticesProcess()
        {
            std::for_each(_vertexOuts.begin(), _vertexOuts.end(), vertexProcessor);
        }
        inline void _verticesPostProcess()
        {
            std::for_each(_vertexOuts.begin(), _vertexOuts.end(), vertexPostProcessor);
        }

        void _assemble()
        {
            auto _end = _vertexOuts.end();
            auto _first = _vertexOuts.begin();
            if (_end == _first)
            {
                return;
            }
            for (auto _last = _vertexOuts.begin(); ++_last != _end;)
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
                _graphObjects.push_back(new _LineType({ *_i, *_j }));
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
                _graphObjects.push_back(new _LineType({ *_i, *_j }));
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
                _graphObjects.push_back(new _LineType({ *_i, *_j }));
            }
            _graphObjects.push_back(new _LineType({ *_i, *_begin }));
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
                _graphObjects.push_back(new _TriangleType({ *_i, *_j, *_k }));
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
                _graphObjects.push_back(new _TriangleType({ *_i, *_j, *_k }));
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
                camara.topDirection.cross(-camara.gazeDirection),
                camara.topDirection,
                -camara.gazeDirection)
                .apply(_perspective)
                .apply(_ortho);

            for (auto _v : _vertexOuts)
            {
                _v->apply(_tm.getTransformMatrix());
            }
        }
        void _perspectiveDivision()
        {
            for (auto _v : _vertexOuts)
            {
                _v->position.divide();
            }
        }
        void _viewportTransform()
        {
            TransformMixer3D _tm;
            _tm.scale(viewport.getWidth() / 2.0, -viewport.getHeight() / 2.0, 1)
                .translate((viewport.getWidth() - 1.0) / 2, (viewport.getHeight() - 1.0) / 2, 0);

            for (auto _v : _vertexOuts)
            {
                _v->apply(_tm.getTransformMatrix());
            }
        }
    };
};

#endif // !_SHADING_UTILITY_
