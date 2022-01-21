#pragma once
#ifndef _DRAWING_UTILITY_
#define _DRAWING_UTILITY_

#include "../lrutility.hpp"
#include "../lrmath.hpp"

namespace lightroom
{
    class GraphObj3D;
    class Line3D;
    class Triangle3D;

    enum class PrimitiveInputType : uint8_t
    {
        NONE, LINES, LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP, TRIANGLE_FAN
    };
    using PxCoordinate = Eigen::Matrix<int, 2, 1>;

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
};

#endif // !_DRAWING_UTILITY_
