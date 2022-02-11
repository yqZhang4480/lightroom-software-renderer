#pragma once
#ifndef _DRAWING_UTILITY_
#define _DRAWING_UTILITY_

#include "../lrutility.hpp"
#include "../lrmath.hpp"

namespace lightroom
{
    class Vertex3D;
    class GraphObj3D;
    template <typename _T> requires std::is_convertible_v<const _T*, const  Vertex3D*> 
    class Line3D;
    template <typename _T> requires std::is_convertible_v<const _T*, const  Vertex3D*> 
    class Triangle3D;

    enum class PrimitiveInputType : uint8_t
    {
        NONE, LINES, LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP, TRIANGLE_FAN
    };
    using PxCoordinate = Eigen::Matrix<int, 2, 1>;
    using UVCoordinate = Eigen::Matrix<Float, 2, 1>;

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
