#pragma once
#ifndef _LRMATH_UTILITY_
#define _LRMATH_UTILITY_

#include "../lrutility.hpp"

namespace lightroom
{
    template <size_t _N>
    using Vector = Eigen::Matrix<Float, _N, 1>;
    template <size_t _N>
    using Matrix = Eigen::Matrix<Float, _N, _N>;
};
#endif // !_LRMATH_UTILITY_