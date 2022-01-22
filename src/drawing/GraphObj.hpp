#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class GraphObj3D
    {
    public:
        virtual void draw(WritableColorMap* _out,
                          DepthBuffer& _depthBuffer,
                          Float _nplain, Float _fplain) const = 0;
        virtual ~GraphObj3D() {}
    };

    class Line3D : public GraphObj3D
    {
        std::array<Vertex3DOut*, 2> _vertices;
    public:
        Line3D(const std::array<Vertex3DOut*, 2>& _vs) : _vertices(_vs)
        {
            for (auto& _v : _vs)
            {
                _v->whenRegisteredByLine(this);
            }
        }
        virtual ~Line3D() {}
        inline bool isVaild() const
        {
            return _vertices[0] != nullptr && _vertices[1] != nullptr;
        }

        virtual void draw(
            WritableColorMap* _outColorMap,
            DepthBuffer& _depthBuffer,
            Float _nplain, Float _fplain) const override final
        {
            auto _v0 = _vertices[0];
            auto _v1 = _vertices[1];

            _sortVertices(_v0, _v1);

            int _x0 = _v0->position[0],
                _y0 = _v0->position[1],
                _x1 = _v1->position[0],
                _y1 = _v1->position[1];

            int _y1_y0 = _y1 - _y0,
                _x1_x0 = _x1 - _x0;
            Float k = Float(_y1_y0) / _x1_x0;

            int _xMin = max(0, min(_x0, _x1)),
                _xMax = min(_outColorMap->getWidth(), max(_x0, _x1)),
                _yMin = max(0, min(_y0, _y1)),
                _yMax = min(_outColorMap->getHeight(), max(_y0, _y1));

            int _x = _x0,
                _y = _y0;

            Float _t = 0;
            Float _depth = _v0->position[2];
            if (k <= -1)
            {
                Float _d = _lineFunction(_x + 0.5, _y - 1);

                Float _dt = sqrt((Float(1) / (k * k) + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);

                for (; _y >= _yMin; _y--)
                {
                    if (_y >= 0 && _y < _outColorMap->getHeight() &&
                        _x >= 0 && _x < _outColorMap->getWidth())
                    {
                        putPixel(_x, _y, _t, _outColorMap, _depthBuffer);
                    }

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
                    _t += _dt;
                }
            }
            else if (k <= 0)
            {
                Float _d = _lineFunction(_x + 1, _y - 0.5);

                Float _dt = sqrt((k * k + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                for (; _x <= _xMax; _x++)
                {
                    if (_y >= 0 && _y < _outColorMap->getHeight() &&
                        _x >= 0 && _x < _outColorMap->getWidth())
                    {
                        putPixel(_x, _y, _t, _outColorMap, _depthBuffer);
                    }

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
                    _t += _dt;
                }
            }
            else if (k <= 1)
            {
                Float _d = _lineFunction(_x + 1, _y + 0.5);

                Float _dt = sqrt((k * k + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                for (; _x <= _xMax; _x++)
                {
                    if (_y >= 0 && _y < _outColorMap->getHeight() &&
                        _x >= 0 && _x < _outColorMap->getWidth())
                    {
                        putPixel(_x, _y, _t, _outColorMap, _depthBuffer);
                    }

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
                    _t += _dt;
                }
            }
            else if (k > 1)
            {
                Float _d = _lineFunction(_x + 0.5, _y + 1);

                Float _dt = sqrt((Float(1) / (k * k) + 1) / (_x1_x0 * _x1_x0 + _y1_y0 * _y1_y0));
                Float _ddepth = _dt * (_v1->position[2] - _v0->position[2]);
                for (; _y <= _yMax; _y++)
                {
                    if (_y >= 0 && _y < _outColorMap->getHeight() &&
                        _x >= 0 && _x < _outColorMap->getWidth())
                    {
                        putPixel(_x, _y, _t, _outColorMap, _depthBuffer);
                    }

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
                    _t += _dt;
                }
            }
        }

        virtual void putPixel(
            int _x, int _y, Float _t,
            WritableColorMap* _colorMap, DepthBuffer& _depthBuffer) const
        {
            int _index = _y * _colorMap->getWidth() + _x;
            Float _depth = _t * _vertices[1]->position[2] + (1 - _t) * _vertices[0]->position[2];
            if (_depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = _depth;
            _colorMap->set(_index, Color(1, 1, 1, 1));
        }

    protected:
        inline Float _lineFunction(Float _x, Float _y) const
        {
            auto& _p0 = _vertices[0]->position;
            auto& _p1 = _vertices[1]->position;
            int _x0 = _p0[0],
                _y0 = _p0[1],
                _x1 = _p1[0],
                _y1 = _p1[1];
            return (_x1 - _x0) * _y - (_y1 - _y0) * _x - _x1 * _y0 + _x0 * _y1;
        }

    private:
        void _sortVertices(lightroom::Vertex3DOut*& _v0, lightroom::Vertex3DOut*& _v1) const
        {
            if (_v0->position[0] > _v1->position[0])
            {
                std::swap(_v0, _v1);
            }
        }
    };
    class Triangle3D : public GraphObj3D
    {
    protected:
        std::array<Vertex3DOut*, 3> _vertices;
    private:
        mutable Float _nplain;
        mutable Float _fplain;

    public:
        Triangle3D(const std::array<Vertex3DOut*, 3>& _vs) : _vertices(_vs)
        {
            for (auto _v : _vs)
            {
                _v->whenRegisteredByTriangle(this);
            }
        }
        virtual ~Triangle3D() {}
        inline bool isVaild() const
        {
            return (_vertices[0] != nullptr) && (_vertices[1] != nullptr) && (_vertices[2] != nullptr);
        }

        Float evaluateAreaSquare() const
        {
            auto& _p0 = _vertices[0]->position;
            auto& _p1 = _vertices[1]->position;
            auto& _p2 = _vertices[2]->position;
            auto a = (_p2 - _p1).norm();
            auto b = (_p2 - _p0).norm();
            auto c = (_p1 - _p0).norm();
            return (a + b + c) * (a + b - c) * (b + c - a) * (c + a - b) / 16;
        }
        Vector<3> evaluateNormal() const
        {
            auto _p0 = _vertices[0]->position.toOrdinary();
            auto&& _p1 = _vertices[1]->position.toOrdinary();
            auto&& _p2 = _vertices[2]->position.toOrdinary();
            return ((_p1 - _p0).cross(_p2 - _p0)).normalized();
        }

        virtual void draw(
            WritableColorMap* _outColorMap,
            DepthBuffer& _depthBuffer,
            Float _nplain, Float _fplain) const override final
        {
            if (!isVaild())
            {
                return;
            }
            this->_nplain = _nplain;
            this->_fplain = _fplain;

            auto& _p0 = _vertices[0]->position;
            auto& _p1 = _vertices[1]->position;
            auto& _p2 = _vertices[2]->position;

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

            Float _dbetax = _d / _M;
            Float _dbetay = -_b / _M;
            Float _dgammax = -_c / _M;
            Float _dgammay = _a / _M;

            for (int _x = _xMin; _x <= _xMax; _x++)
            {
                Float __beta = _beta;
                Float __gamma = _gamma;
                for (int _y = _yMin; _y <= _yMax; _y++)
                {
                    if ((unsigned)_y >= _outColorMap->getHeight() ||
                        (unsigned)_x >= _outColorMap->getWidth())
                    {
                        continue;
                    }
                    __beta += _dbetay;
                    __gamma += _dgammay;

                    if (__beta < 0 || __beta > 1)
                    {
                        continue;
                    }
                    if (__gamma < 0 || __gamma > 1 || __beta + __gamma > 1)
                    {
                        continue;
                    }
                    Float __alpha = 1 - __beta - __gamma;

                    putPixel(_x, _y, __alpha, __beta, __gamma, _outColorMap, _depthBuffer);
                }
                _beta += _dbetax;
                _gamma += _dgammax;
            }
        }
    protected:
        template <typename _Value>
        inline _Value linearInterpolation(Float _alpha, Float _beta, Float _gamma,
                                   const std::function<_Value(const Vertex3DOut*)>& _func) const
        {
            return _alpha * _func(_vertices[0]) +
                _beta * _func(_vertices[1]) +
                _gamma * _func(_vertices[2]);
        }
        template <typename _Value>
        inline _Value perspectiveInterpolation(Float _alpha, Float _beta, Float _gamma,
                                        const std::function<_Value(const Vertex3DOut*)>& _func) const
        {
            Float _1_Z0 = _nplain + _fplain - _vertices[0]->position[2] * (_nplain - _fplain);
            Float _1_Z1 = _nplain + _fplain - _vertices[1]->position[2] * (_nplain - _fplain);
            Float _1_Z2 = _nplain + _fplain - _vertices[2]->position[2] * (_nplain - _fplain);

            Float _1_Zp =
                (_nplain + _fplain -
                 linearInterpolation<Float>(_alpha, _beta, _gamma,
                                            [](const Vertex3DOut* _v)
                                            {
                                                return _v->position[2];
                                            }) * (_nplain - _fplain));
            return
                _func(_vertices[0]) * _alpha *  (_1_Z0 / _1_Zp) +
                _func(_vertices[1]) * _beta  *  (_1_Z1 / _1_Zp) +
                _func(_vertices[2]) * _gamma *  (_1_Z2 / _1_Zp);
        }
        virtual void putPixel(
            int _x, int _y, Float _alpha, Float _beta, Float _gamma,
            WritableColorMap* _colorMap, DepthBuffer& _depthBuffer) const
        {
            int _index = _y * _colorMap->getWidth() + _x;
            Float _depth = linearInterpolation<Float>(
                _alpha, _beta, _gamma,
                [](const Vertex3DOut* _v)
                {
                    return _v->position[2];
                });
            if (_depth <= _depthBuffer[_index])
            {
                return;
            }
            _depthBuffer[_index] = _depth;
            _colorMap->set(_index, Color(1, 1, 1, 1));
        }

    };

};