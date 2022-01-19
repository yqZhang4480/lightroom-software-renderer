#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class GraphObj3D
    {
    public:
        virtual void draw(WritableColorMap* _out,
                          OverwriteMask& _mask,
                          DepthBuffer& _depthBuffer,
                          ReferenceBuffer& _refBuffer,
                          Float _nplain, Float _fplain) const = 0;
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
            WritableColorMap* _outColorMap,
            OverwriteMask& _mask,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer,
            Float _nplain, Float _fplain) const override
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
                _xMax = min(_outColorMap->getWidth(), max(_x0, _x1)),
                _yMin = max(0, min(_y0, _y1)),
                _yMax = min(_outColorMap->getHeight(), max(_y0, _y1));
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
                    _putPixel(_x, _y, _depth, _color, _outColorMap, _mask, _depthBuffer, _refBuffer);

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
                    _putPixel(_x, _y, _depth, _color, _outColorMap, _mask, _depthBuffer, _refBuffer);

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
                    _putPixel(_x, _y, _depth, _color, _outColorMap, _mask, _depthBuffer, _refBuffer);

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
                    _putPixel(_x, _y, _depth, _color, _outColorMap, _mask, _depthBuffer, _refBuffer);

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
            int _x, int _y, Float _depth, const Color& _color,
            WritableColorMap* _colorMap, OverwriteMask& _mask,
            DepthBuffer& _depthBuffer, ReferenceBuffer& _refBuffer) const
        {
            if (_y < 0 || _y >= _colorMap->getHeight() ||
                _x < 0 || _x >= _colorMap->getWidth())
            {
                return;
            }

            int _index = _y * _colorMap->getWidth() + _x;
            if (_depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = _depth;
            _refBuffer[_index] = this;
            _mask[_index] = true;
            _colorMap->set(_index, _color);
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
            auto _p0 = _vertexs[0]->position.toOrdinary();
            auto&& _p1 = _vertexs[1]->position.toOrdinary();
            auto&& _p2 = _vertexs[2]->position.toOrdinary();
            return ((_p1 - _p0).cross(_p2 - _p0)).normalized();
        }

        virtual void draw(
            WritableColorMap* _outColorMap,
            OverwriteMask& _mask,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer,
            Float _nplain, Float _fplain) const override
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
                _xMax = min(_outColorMap->getWidth(), _vpXMax),
                _yMin = max(0, _vpYMin),
                _yMax = min(_outColorMap->getHeight(), _vpYMax);

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

            Float _z0 = _vertexs[0]->position[2];
            Float _z1 = _vertexs[1]->position[2];
            Float _z2 = _vertexs[2]->position[2];
            Float _1_Z0 = _nplain + _fplain - _z0 * (_nplain - _fplain);
            Float _1_Z1 = _nplain + _fplain - _z1 * (_nplain - _fplain);
            Float _1_Z2 = _nplain + _fplain - _z2 * (_nplain - _fplain);

            Float _zp = _z0 * _alpha + _z1 * _beta + _z2 * _gamma;

            Float _dbetax = _d / _M;
            Float _dbetay = -_b / _M;
            Float _dgammax = -_c / _M;
            Float _dgammay = _a / _M;

            Float _dzx = _dbetax * (_z1 - _z0) + _dgammax * (_z2 - _z0);
            Float _dzy = _dbetay * (_z1 - _z0) + _dgammay * (_z2 - _z0);

            for (int _x = _xMin; _x <= _xMax; _x++)
            {
                Float __beta = _beta;
                Float __gamma = _gamma;
                Float __zp = _zp;
                for (int _y = _yMin; _y <= _yMax; _y++)
                {
                    if ((unsigned)_y >= _outColorMap->getHeight() ||
                        (unsigned)_x >= _outColorMap->getWidth())
                    {
                        continue;
                    }
                    __beta += _dbetay;
                    __gamma += _dgammay;
                    __zp += _dzy;

                    if (__beta < 0 || __beta > 1)
                    {
                        continue;
                    }
                    if (__gamma < 0 || __gamma > 1 || __beta + __gamma > 1)
                    {
                        continue;
                    }

                    Float __alpha = 1 - __beta - __gamma;
                    Float _1_Zp = (_nplain + _fplain - __zp * (_nplain - _fplain));
                    Float __u =
                        _vertexs[0]->texturePosition[0] * __alpha / (_1_Zp / _1_Z0) +
                        _vertexs[1]->texturePosition[0] * __beta  / (_1_Zp / _1_Z1) +
                        _vertexs[2]->texturePosition[0] * __gamma / (_1_Zp / _1_Z2);
                    Float __v =
                        _vertexs[0]->texturePosition[1] * __alpha / (_1_Zp / _1_Z0) +
                        _vertexs[1]->texturePosition[1] * __beta  / (_1_Zp / _1_Z1) +
                        _vertexs[2]->texturePosition[1] * __gamma / (_1_Zp / _1_Z2);

                    if (__u < 0 || __v < 0 ||
                        __u > _vertexs[0]->texture->getWidth() ||
                        __v >= _vertexs[0]->texture->getHeight())
                    {
                        continue;
                    }
                    auto __color = _vertexs[0]->texture->get(PxCoordinate{ __u, __v });
                    _putPixel(_x, _y, __zp, __color, _outColorMap, _mask, _depthBuffer, _refBuffer);
                }
                _beta += _dbetax;
                _gamma += _dgammax;
                _zp += _dzx;
            }
        }
    protected:
        void _putPixel(
            int _x, int _y, Float __depth, const Color& __color,
            WritableColorMap* _colorMap, OverwriteMask& _mask,
            DepthBuffer& _depthBuffer,
            ReferenceBuffer& _refBuffer) const
        {
            int _index = _y * _colorMap->getWidth() + _x;

            if (__depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = __depth;
            _refBuffer[_index] = this;
            _mask[_index] = true;
            _colorMap->set(_index, __color);
        }
    };
};