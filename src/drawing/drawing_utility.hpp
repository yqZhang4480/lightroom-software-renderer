#pragma once
#ifndef _DRAWING_UTILITY_
#define _DRAWING_UTILITY_

#include "../lrutility.hpp"
#include "../lrmath.hpp"

namespace lightroom
{
    class NormalizedColor
    {
    public:
        NormalizedColor() : _rgba{ 0,0,0,1 } {}
        NormalizedColor(Float _r, Float _g, Float _b, Float _a = 1) : _rgba{ _r, _g, _b, _a } {}
        NormalizedColor(COLORREF _rgb) :
            _rgba{
                ((_rgb & 0x00ff0000) >> 16) / Float(256),
                ((_rgb & 0x0000ff00) >> 8) / Float(256),
                (_rgb & 0x000000ff) / Float(256),
                1
        } {}

        explicit operator COLORREF() const
        {
            auto r = static_cast<uint8_t>((_rgba[0] * 256) <= 255 ? (_rgba[0] * 256) : 255);
            auto g = static_cast<uint8_t>((_rgba[1] * 256) <= 255 ? (_rgba[1] * 256) : 255);
            auto b = static_cast<uint8_t>((_rgba[2] * 256) <= 255 ? (_rgba[2] * 256) : 255);
            return b | (((uint16_t)g) << 8) | (((uint32_t)r) << 16);
        }
        COLORREF toRGBColor() const
        {
            return static_cast<COLORREF>(*this);
        }

        inline Float& operator[](size_t _index)
        {
            return _rgba[_index];
        }
        inline Float operator[](size_t _index) const
        {
            return _rgba[_index];
        }
        inline NormalizedColor& operator+= (const NormalizedColor& _nc2)
        {
            _rgba[0] += _nc2._rgba[0];
            _rgba[1] += _nc2._rgba[1];
            _rgba[2] += _nc2._rgba[2];
            _rgba[3] += _nc2._rgba[3];
            return *this;
        }
        inline NormalizedColor& operator*= (Float _factor)
        {
            _rgba[0] *= _factor;
            _rgba[1] *= _factor;
            _rgba[2] *= _factor;
            _rgba[3] *= _factor;
            return *this;
        }
        inline friend NormalizedColor operator+ (NormalizedColor _nc1, const NormalizedColor& _nc2)
        {
            return _nc1 += _nc2;
        }
        inline friend NormalizedColor operator* (NormalizedColor _nc, Float _factor)
        {
            return _nc *= _factor;
        }
        inline friend NormalizedColor operator* (Float _factor, NormalizedColor _nc)
        {
            return _nc *= _factor;
        }
    protected:
        Float _rgba[4];
    };

    class GraphObj3D;
    class Triangle3D;
    using PxCoordinate = Eigen::Matrix<int, 2, 1>;
    using ViewportCoordinate = Eigen::Matrix<Float, 2, 1>;
    class DepthBuffer : public std::vector<Float>
    {
    public:
        template <typename... _Args>
        DepthBuffer(_Args&&... _args) : std::vector<Float>(_args...) {}
        void clear()
        {
            std::fill(begin(), end(), -1);
        }
    };
    class ReferenceBuffer : public std::vector<GraphObj3D const*>
    {
    public:
        template <typename... _Args>
        ReferenceBuffer(_Args&&... _args) : std::vector<GraphObj3D const*>(_args...) {}
        void clear()
        {
            std::fill(begin(), end(), nullptr);
        }
    };
    class OverwriteMask : public std::vector<int>
    {
    public:
        template <typename... _Args>
        OverwriteMask(_Args&&... _args) : std::vector<int>(_args...) {}
        void clear()
        {
            std::fill(begin(), end(), false);
        }
    };

    class NormalizedColorMap
    {
    protected:
        PxCoordinate _size;
        std::vector<NormalizedColor> _data;
    public:
        NormalizedColorMap(const PxCoordinate& __size = { 0,0 }, const NormalizedColor& _color = { 0,0,0,1 }) :
            _size(__size), _data(__size[0] * __size[1])
        {
            for (auto& _d : _data)
            {
                _d = _color;
            }
        }
        NormalizedColor& operator[] (size_t _index)
        {
            return _data[_index];
        }
        const NormalizedColor& operator[] (size_t _index) const
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
        void clear()
        {
            std::fill(_data.begin(), _data.end(), NormalizedColor{ 0,0,0,1 });
        }
    };

    class Viewport
    {
    public:
        Viewport(LPRECT lpRect = nullptr, const NormalizedColor& _backgroundColor = { 0,0,0,1 }, const int _flag = EW_SHOWCONSOLE)
        {
            SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
            int max_w = GetSystemMetrics(SM_CXSCREEN);
            int max_h = GetSystemMetrics(SM_CYSCREEN);
            int left, top;
            if (lpRect == nullptr)
            {
                left = 0;
                top = 0;
                _width = max_w;
                _height = max_h;
            }
            else
            {
                left = lpRect->left;
                top = lpRect->top;
                _width = lpRect->right - lpRect->left;
                _height = lpRect->bottom - lpRect->top;
                if (_width <= 0)
                {
                    _width = max_w - 2 * left;
                }
                if (_height <= 0)
                {
                    _height = max_h - 2 * top;
                }
            }

            background = NormalizedColorMap(PxCoordinate{ _width, _height }, _backgroundColor);
            HWND _hwnd = initgraph(_width, _height, _flag);


            LONG _winStyle = GetWindowLong(_hwnd, GWL_STYLE);
            SetWindowLong(_hwnd, GWL_STYLE,
                          (_winStyle | WS_POPUP) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_BORDER);
            SetWindowPos(_hwnd, HWND_TOP, left, top, _width, _height, 0);

            BeginBatchDraw();

            if (lpRect != nullptr)
            {
                lpRect->left = left;
                lpRect->right = left + _width;
                lpRect->top = top;
                lpRect->bottom = top + _height;
            }
        }
        ~Viewport() = default;
        Viewport(const Viewport&) = delete;
        Viewport& operator=(const Viewport&) = delete;

        int getWidth()
        {
            return _width;
        }
        int getHeight()
        {
            return _height;
        }
        void print(const NormalizedColorMap& _ncm, const OverwriteMask& _mask, IMAGE* _outDevice = NULL) const
        {
            auto _imgBuffer = GetImageBuffer(_outDevice);
            for (size_t _i = 0; _i < static_cast<size_t>(_width) * _height; _i++)
            {
                _imgBuffer[_i] =
                    (_mask[_i] == false) ? background[_i].toRGBColor() : _ncm[_i].toRGBColor();
            }
            FlushBatchDraw();
        }
        NormalizedColorMap background;
    private:
        int _width;
        int _height;

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

    enum class GraphObjType : uint8_t
    {
        NONE, LINES, LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP, TRIANGLE_FAN
    };
    class Vertex3DIn
    {
    public:
        friend class PipelineManager;
        Homogeneous<4> position;
        Vertex3DIn(const Vector<3>& position) :
            position(position), _graphObjType(GraphObjType::NONE) {}
        GraphObjType _graphObjType;
        Vertex3DIn(GraphObjType _graphObjType, const Homogeneous<4>& position) :
            position(position), _graphObjType(_graphObjType) {}
    };
    class Vertex3DOut
    {
    public:
        friend class PipelineManager;
        Vertex3DOut(
            const Homogeneous<4>& position = Homogeneous<4>(Vector<3>{ 0, 0, 0 }),
            const Vector<3>& normal = { 0, 0, 0 },
            const NormalizedColor& color = { 0, 0, 0, 1 }) :
            _graphObjType(GraphObjType::NONE), position(position), color(color), normal(normal) {}

        Vertex3DOut& apply(const Matrix<4>& _transformMatrix)
        {
            position.apply(_transformMatrix);
            return *this;
        }
        std::vector<Triangle3D const*> relatedTriangles;
        Homogeneous<4> position;
        Vector<3> normal;
        NormalizedColor color;
        Vertex3DOut(GraphObjType _graphObjType,
                    const Homogeneous<4>& position = Homogeneous<4>(Vector<3>{ 0, 0, 0 }),
                    const Vector<3>& normal = { 0, 0, 0 },
                    const NormalizedColor& color = { 0, 0, 0, 1 }) :
            _graphObjType(_graphObjType), position(position), color(color), normal(normal) {}
        GraphObjType _graphObjType;
    };

    
    class GraphObj3D
    {
    public:
        virtual void draw(NormalizedColorMap&,
                          OverwriteMask&,
                          DepthBuffer&,
                          ReferenceBuffer&) const = 0;
        virtual ~GraphObj3D() {}
    };
    class Line3D : public GraphObj3D
    {
        std::array<Vertex3DOut*, 2> _vertexs;
    public:
        Line3D(const std::array<Vertex3DOut*, 2>& _v) : _vertexs(_v)
        {}
        virtual ~Line3D() {}
        inline bool isVaild() const
        {
            return _vertexs[0] != nullptr && _vertexs[1] != nullptr;
        }

        virtual void draw(
            NormalizedColorMap& _ncm,
            OverwriteMask& _mask,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer) const override
        {
            auto _v0 = _vertexs[0];
            auto _v1 = _vertexs[1];
            if (_v0->position[0] > _v1->position[0])
            {
                std::swap(_v0, _v1);
            }
            int _x0 = _v0->position[0],
                _y0 = _v0->position[1],
                _x1 = _v1->position[0],
                _y1 = _v1->position[1];
            int _y1_y0 = _y1 - _y0,
                _x1_x0 = _x1 - _x0;
            int _xMin = max(0, min(_x0, _x1)),
                _xMax = min(_ncm.getWidth(), max(_x0, _x1)),
                _yMin = max(0, min(_y0, _y1)),
                _yMax = min(_ncm.getHeight(), max(_y0, _y1));
            int _x = _x0,
                _y = _y0;
            Float k = Float(_y1_y0) / _x1_x0;

            Float _depth = _v0->position[2];
            auto _color = _v0->color;
            if (k <= -1)
            {
                Float _d = _lineFunction(_x + 0.5, _y - 1);

                Float _dt = sqrt((Float(1) / (k * k) + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                auto _dcolor = _dt * (_v1->color + -1 * _v0->color);

                for (; _y >= _yMin; _y--)
                {
                    _putPixel(_x, _y, _depth, _color, _ncm, _mask, _depthBuffer, _refBuffer);

                    if (_d < 0)
                    {
                        _x++;
                        _d += -_x1_x0 - _y1_y0;
                    }
                    else
                    {
                        _d += -_x1_x0;
                    }
                    _depth += _ddepth;
                    _color += _dcolor;
                }
            }
            else if (k <= 0)
            {
                Float _d = _lineFunction(_x + 1, _y - 0.5);

                Float _dt = sqrt((k * k + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                auto _dcolor = _dt * (_v1->color + -1 * _v0->color);
                for (; _x <= _xMax; _x++)
                {
                    _putPixel(_x, _y, _depth, _color, _ncm, _mask, _depthBuffer, _refBuffer);

                    if (_d > 0)
                    {
                        _y--;
                        _d += -_x1_x0 - _y1_y0;
                    }
                    else
                    {
                        _d += -_y1_y0;
                    }
                    _depth += _ddepth;
                    _color += _dcolor;
                }
            }
            else if (k <= 1)
            {
                Float _d = _lineFunction(_x + 1, _y + 0.5);

                Float _dt = sqrt((k * k + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                auto _dcolor = _dt * (_v1->color + -1 * _v0->color);
                for (; _x <= _xMax; _x++)
                {
                    _putPixel(_x, _y, _depth, _color, _ncm, _mask, _depthBuffer, _refBuffer);

                    if (_d < 0)
                    {
                        _y++;
                        _d += _x1_x0 - _y1_y0;
                    }
                    else
                    {
                        _d += -_y1_y0;
                    }
                    _depth += _ddepth;
                    _color += _dcolor;
                }
            }
            else if (k > 1)
            {
                Float _d = _lineFunction(_x + 0.5, _y + 1);

                Float _dt = sqrt((Float(1) / (k * k) + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                auto _dcolor = _dt * (_v1->color + -1 * _v0->color);
                for (; _y <= _yMax; _y++)
                {
                    _putPixel(_x, _y, _depth, _color, _ncm, _mask, _depthBuffer, _refBuffer);

                    if (_d > 0)
                    {
                        _x++;
                        _d += _x1_x0 - _y1_y0;
                    }
                    else
                    {
                        _d += _x1_x0;
                    }
                    _depth += _ddepth;
                    _color += _dcolor;
                }
            }
        }

    protected:
        inline void _putPixel(
            int _x, int _y, Float _depth, const NormalizedColor& _color,
            NormalizedColorMap& _ncm, OverwriteMask& _mask,
            DepthBuffer& _depthBuffer, ReferenceBuffer& _refBuffer) const
        {
            if (_y < 0 || _y >= _ncm.getHeight() ||
                _x < 0 || _x >= _ncm.getWidth())
            {
                return;
            }

            int _index = _y * _ncm.getWidth() + _x;
            if (_depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = _depth;
            _refBuffer[_index] = this;
            _mask[_index] = true;
            _ncm[_index] = _color;
        }

        inline Float _lineFunction(Float _x, Float _y) const
        {
            auto& _p0 = _vertexs[0]->position;
            auto& _p1 = _vertexs[1]->position;
            int _x0 = _p0[0],
                _y0 = _p0[1],
                _x1 = _p1[0],
                _y1 = _p1[1];
            return (_x1 - _x0) * _y - (_y1 - _y0) * _x - _x1 * _y0 + _x0 * _y1;
        }
    };
    class Triangle3D : public GraphObj3D
    {
    protected:
        std::array<Vertex3DOut*, 3> _vertexs;
        class Vertex3DPointerYComparer
        {
        public:
            bool operator()(const Vertex3DOut* _left, const Vertex3DOut* _right)
            {
                return _left->position[1] < _right->position[1];
            }
        };

    public:
        Triangle3D(const std::array<Vertex3DOut*, 3>& _v) : _vertexs(_v)
        {
            _v[0]->relatedTriangles.push_back(this);
            _v[1]->relatedTriangles.push_back(this);
            _v[2]->relatedTriangles.push_back(this);
        }
        virtual ~Triangle3D() {}
        inline bool isVaild() const
        {
            return (_vertexs[0] != nullptr) && (_vertexs[1] != nullptr) && (_vertexs[2] != nullptr);
        }

        Float evaluateAreaSquare() const
        {
            auto& _p0 = _vertexs[0]->position;
            auto& _p1 = _vertexs[1]->position;
            auto& _p2 = _vertexs[2]->position;
            auto a = (_p2 - _p1).norm();
            auto b = (_p2 - _p0).norm();
            auto c = (_p1 - _p0).norm();
            return (a + b + c) * (a + b - c) * (b + c - a) * (c + a - b) / 16;
        }
        Vector<3> evaluateNormal() const
        {
            auto _p0 = _vertexs[0]->position.toOrdinaryCoordinate();
            auto&& _p1 = _vertexs[1]->position.toOrdinaryCoordinate();
            auto&& _p2 = _vertexs[2]->position.toOrdinaryCoordinate();
            return ((_p1 - _p0).cross(_p2 - _p0)).normalized();
        }

        virtual void draw(
            NormalizedColorMap& _ncm,
            OverwriteMask& _mask,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer) const override
        {
            if (!isVaild())
            {
                return;
            }

            auto& _p0 = _vertexs[0]->position;
            auto& _p1 = _vertexs[1]->position;
            auto& _p2 = _vertexs[2]->position;

            int _x0 = _p0[0],
                _x1 = _p1[0],
                _x2 = _p2[0],
                _y0 = _p0[1],
                _y1 = _p1[1],
                _y2 = _p2[1];

            int _vpXMin = min(min(_x0, _x1), _x2),
                _vpXMax = max(max(_x0, _x1), _x2),
                _vpYMin = min(min(_y0, _y1), _y2),
                _vpYMax = max(max(_y0, _y1), _y2);

            int _xMin = max(0, _vpXMin),
                _xMax = min(_ncm.getWidth(), _vpXMax),
                _yMin = max(0, _vpYMin),
                _yMax = min(_ncm.getHeight(), _vpYMax);

            Float _a = _x1 - _x0,
                _b = _x2 - _x0,
                _c = _y1 - _y0,
                _d = _y2 - _y0,
                _e = _xMin - _x0,
                _f = _yMin - _y0;
            Float _M = _a * _d - _b * _c;
            Float _beta = (_e * _d - _b * _f) / _M;
            Float _gamma = (_a * _f - _c * _e) / _M;
            Float _alpha = 1 - _beta - _gamma;

            Float _depth = _vertexs[0]->position[2] * _alpha +
                _vertexs[1]->position[2] * _beta +
                _vertexs[2]->position[2] * _gamma;
            auto _color = NormalizedColor{
                _vertexs[0]->color * _alpha +
                _vertexs[1]->color * _beta +
                _vertexs[2]->color * _gamma };

            Float _dbetax = _d / _M;
            Float _dbetay = -_b / _M;
            Float _dgammax = -_c / _M;
            Float _dgammay = _a / _M;

            Float _ddepthx =
                _dbetax * (_vertexs[1]->position[2] - _vertexs[0]->position[2]) +
                _dgammax * (_vertexs[2]->position[2] - _vertexs[0]->position[2]);
            Float _ddepthy =
                _dbetay * (_vertexs[1]->position[2] - _vertexs[0]->position[2]) +
                _dgammay * (_vertexs[2]->position[2] - _vertexs[0]->position[2]);
            auto _dcolorx =
                _dbetax * (_vertexs[1]->color + -1 * _vertexs[0]->color) +
                _dgammax * (_vertexs[2]->color + -1 * _vertexs[0]->color);
            auto _dcolory =
                _dbetay * (_vertexs[1]->color + -1 * _vertexs[0]->color) +
                _dgammay * (_vertexs[2]->color + -1 * _vertexs[0]->color);

            for (int _x = _xMin; _x <= _xMax; _x++)
            {
                Float __beta = _beta;
                Float __gamma = _gamma;
                Float __depth = _depth;
                auto __color = _color;
                for (int _y = _yMin; _y <= _yMax; _y++)
                {
                    if ((unsigned)_y >= _ncm.getHeight() || (unsigned)_x >= _ncm.getWidth())
                    {
                        continue;
                    }
                    __beta += _dbetay;
                    __gamma += _dgammay;
                    __depth += _ddepthy;
                    __color += _dcolory;

                    if (__beta < 0 || __beta > 1)
                    {
                        continue;
                    }
                    if (__gamma < 0 || __gamma > 1 || __beta + __gamma > 1)
                    {
                        continue;
                    }
                    _putPixel(_x, _y, __depth, __color, _ncm, _mask, _depthBuffer, _refBuffer);
                }
                _beta += _dbetax;
                _gamma += _dgammax;
                _depth += _ddepthx;
                _color += _dcolorx;
            }
        }
    protected:
        void _putPixel(
            int _x, int _y, Float __depth, const NormalizedColor& __color,
            NormalizedColorMap& _ncm, OverwriteMask& _mask,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer) const
        {
            int _index = _y * _ncm.getWidth() + _x;

            if (__depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = __depth;
            _refBuffer[_index] = this;
            _mask[_index] = true;
            _ncm[_index] = __color;
        }
    };

};

#endif // !_DRAWING_UTILITY_
