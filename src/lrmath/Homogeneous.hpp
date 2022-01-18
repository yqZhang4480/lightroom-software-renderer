#pragma once
#include "lrmath_utility.hpp"

namespace lightroom
{
    template <size_t _N> requires (_N - 1 > 0)
    class Homogeneous : public Vector<_N>
    {
    public:
        Homogeneous(const Vector<_N - 1>& _normalCoordinate)
        {
            _overwrite_vector<_N - 1, _N>(_normalCoordinate, *this);
            (*this)[_N - 1] = 1;
        }
        Homogeneous(const Vector<_N>& _homogeneousCoordinate) : Vector<_N>(_homogeneousCoordinate) {}

        inline void divide()
        {
            *this /= (*this)[_N - 1];
        }
        Vector<_N - 1> toOrdinaryCoordinate()
        {
            Vector<_N - 1> _ret;
            if ((*this)[_N - 1] != 0)
            {
                divide();
            }
            _overwrite_vector<_N, _N - 1>(*this, _ret);
            return _ret;
        }
        using Vector<_N>::operator=;
        Homogeneous<_N>& apply(const Matrix<_N>& _transformMatrix)
        {
            *this = _transformMatrix * (*this);
            return *this;
        }

    private:
        template <size_t _SRC_SIZE, size_t _DST_SIZE>
        inline static void _overwrite_vector(
            const Vector<_SRC_SIZE>& _source, Vector<_DST_SIZE>& _destination)
        {
            for (size_t i = 0; i < _SRC_SIZE && i < _DST_SIZE; i++)
            {
                _destination[i] = _source[i];
            }
        }
    };
}