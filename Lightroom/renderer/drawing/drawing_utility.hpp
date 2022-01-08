#pragma once
#ifndef _DRAWING_UTILITY_
#define _DRAWING_UTILITY_

#include "../lrutility.hpp"
#include "../lrmath.hpp"

namespace lightroom
{
    class NormalizedColor : protected Eigen::Matrix<Float, 4, 1>
    {
    public:
        NormalizedColor() : Matrix{ 0,0,0,1 } {}
        NormalizedColor(Float _r, Float _g, Float _b, Float _a = 1) : Matrix{_r, _g, _b, _a} {}
        NormalizedColor(COLORREF _rgb) :
            Matrix{
                ((_rgb & 0x00ff0000) >> 16) / Float(256),
                ((_rgb & 0x0000ff00) >> 8)  / Float(256),
                ( _rgb & 0x000000ff)        / Float(256),
                1
            } {}

        explicit operator COLORREF() const
        {
            auto r = static_cast<uint8_t>(((*this)[0] * 256) <= 255 ? ((*this)[0] * 256) : 255);
            auto g = static_cast<uint8_t>(((*this)[1] * 256) <= 255 ? ((*this)[1] * 256) : 255);
            auto b = static_cast<uint8_t>(((*this)[2] * 256) <= 255 ? ((*this)[2] * 256) : 255);
            return b | (((uint16_t)g) << 8) | (((uint32_t)r) << 16);
        }
        COLORREF toRGBColor() const
        {
            return static_cast<COLORREF>(*this);
        }

        friend NormalizedColor operator+ (const NormalizedColor& _nc1, const NormalizedColor& _nc2)
        {
            return _nc1 + _nc2;
        }
        friend NormalizedColor operator* (const NormalizedColor& _nc, Float _factor)
        {
            return _nc * _factor;
        }
        friend NormalizedColor operator* (Float _factor, const NormalizedColor& _nc)
        {
            return _nc * _factor;
        }
    };

    using PxCoordinate = Eigen::Matrix<int, 2, 1>;
    using ViewportCoordinate = Eigen::Matrix<Float, 2, 1>;
    using DepthBuffer = std::vector<Float>;
    using ReferenceBuffer = std::vector<GraphObj3D const*>;

    class Viewport
    {
    public:
        Viewport() = default;
    };

    class NormalizedColorMap
    {
    protected:
        PxCoordinate _size;
        std::vector<NormalizedColor> _data;
    public:
        NormalizedColor& operator[] (size_t _index)
        {
            return _data[_index];
        }
        int getWidth() const
        {
            return _size[0];
        }
        int getHeight() const
        {
            return _size[1];
        }
    };

    /*class Printer
    {
    public:
        PxCoordinate leftTop;
        PxCoordinate size;

        template<typename... _Args>
        Printer(int _left, int _top, int _right, int _bottom) :
            leftTop{ _left, _top }, size{ _right - _left, _bottom - _top } {}

        void clear() const
        {
            cleardevice();
        }
        void flush() const
        {
            FlushBatchDraw();
        }
        void print(
            const RGBColorMap& _rgbMap, IMAGE* pImg = NULL) const
        {
            for (int _row = 0; _row < min(size[1], _bottomBound); _row++)
            {
                for (int _col = 0; _col < min(size[0], _rightBound); _col++)
                {
                    int _colorMapIndex = _row * size[0] + _col;
                    int _screenX = _row + leftTop[0];
                    int _screenY = _col + leftTop[1];
                    if (_screenX < 0 || _screenY < 0)
                    {
                        continue;
                    }
                    GetImageBuffer(pImg)[_screenX + _screenY]
                }
            }
        }
    };*/

    class Vertex3D
    {
    public:
        Vertex3D() = default;
        Vertex3D(
            const Homogeneous<4>& position,
            const Homogeneous<4>& normal,
            const NormalizedColor& color = { 0, 0, 0 }) :
            position(position), color(color), normal(normal) {}

        Vertex3D& apply(const Matrix<4>& _transformMatrix)
        {
            position = _transformMatrix * position;
            return *this;
        }
        std::vector<Triangle3D const*> relatedTriangles;

        Homogeneous<4> position;
        Homogeneous<4> normal;
        NormalizedColor color;
    };
    class GraphObj3D
    {
    public:
        virtual void draw(NormalizedColorMap&, DepthBuffer&, ReferenceBuffer&) const = 0;
        virtual ~GraphObj3D() {}
    };
    class Triangle3D : public GraphObj3D
    {
    protected:
        std::array<Vertex3D*, 3> _vertexs;

    public:
        Triangle3D(const std::array<Vertex3D*, 3>& _v) : _vertexs(_v)
        {
            _v[0]->relatedTriangles.push_back(this);
            _v[1]->relatedTriangles.push_back(this);
            _v[2]->relatedTriangles.push_back(this);
        }

        inline bool isVaild() const
        {
            return (_vertexs[0] != nullptr) && (_vertexs[1] != nullptr) && (_vertexs[2] != nullptr);
        }
        virtual void draw(
            NormalizedColorMap& _ncm,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer) const override
        {
            if (!isVaild())
            {
                return;
            }

            int _x0 = _vertexs[0]->position[0],
                _x1 = _vertexs[1]->position[0],
                _x2 = _vertexs[2]->position[0],
                _y0 = _vertexs[0]->position[1],
                _y1 = _vertexs[1]->position[1],
                _y2 = _vertexs[2]->position[1];
            int _x_min = max(0, min(min(_x0, _x1), _x2)),
                _x_max = min(_ncm.getWidth(), max(max(_x0, _x1), _x2)),
                _y_min = max(0, min(min(_y0, _y1), _y2)),
                _y_max = min(_ncm.getHeight(), max(max(_y0, _y1), _y2));

            for (int _x = _x_min; _x <= _x_max; _x++)
            {
                for (int _y = _y_min; _y <= _y_max; _y++)
                {
                    if ((unsigned)_y >= _ncm.getHeight() || (unsigned)_x >= _ncm.getWidth())
                    {
                        continue;
                    }
                    Float _a = _x1 - _x0,
                          _b = _x2 - _x0,
                          _c = _y1 - _y0,
                          _d = _y2 - _y0,
                          _e = _x - _x0,
                          _f = _y - _y0;
                    Float _M = _a * _d - _b * _c;
                    Float _beta = (_e * _d - _b * _f) / _M;
                    if (_beta < 0 || _beta > 1)
                    {
                        continue;
                    }
                    Float _gamma = (_a * _f - _c * _e) / _M;
                    if (_gamma < 0 || _gamma > 1 || _beta + _gamma > 1)
                    {
                        continue;
                    }
                    _evaluateColorAndDepth(_x, _y, _beta, _gamma, _ncm, _depthBuffer, _refBuffer);
                }
            }
        }

    protected:
        inline void _evaluateColorAndDepth(
            int _x, int _y,
            Float& _beta, Float& _gamma,
            NormalizedColorMap& _ncm,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer) const
        {
            Float _alpha = 1 - _beta - _gamma;
            int _index = _y * _ncm.getWidth() + _x;
            Float _depth = _vertexs[0]->position[2] * _alpha +
                _vertexs[1]->position[2] * _beta +
                _vertexs[2]->position[2] * _gamma;

            if (_depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = _depth;
            _refBuffer[_index] = this;

            _ncm[_index] = NormalizedColor {
                _vertexs[0]->color * _alpha +
                _vertexs[1]->color * _beta +
                _vertexs[2]->color * _gamma };
        }
    };

};

#endif // !_DRAWING_UTILITY_
