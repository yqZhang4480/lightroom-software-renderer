#pragma once
#include "lrmath_utility.hpp"

namespace lightroom
{
    class Homogeneous final : public Vector<4>
    {
    public:
        // 由笛卡尔坐标构造齐次坐标
        inline Homogeneous(const Vector<3>& _cartesianCoordinate, Float _w = 1);

        // 对各分量除以w
        inline void divide();

        // 由齐次坐标获得笛卡尔坐标
        inline Vector<3> toCartesian();
        inline Vector<3> toCartesian() const;
        
        // 应用变换
        inline Homogeneous& apply(const TransformMixer3D& _mixer);

    private:
        template <size_t _SRC_SIZE, size_t _DST_SIZE>
        inline static void _overwrite_vector(
            const Vector<_SRC_SIZE>& _source, Vector<_DST_SIZE>& _destination);
    };

    inline Homogeneous::Homogeneous(const Vector<3>& _cartesianCoordinate, Float _w)
    {
        _overwrite_vector<3, 4>(_cartesianCoordinate, *this);
        (*this)[3] = _w;
    }

    inline void Homogeneous::divide()
    {
        if ((*this)[3] == Float(0))
        {
            return;
        }
        *this /= (*this)[3];
    }
    inline Vector<3> Homogeneous::toCartesian()
    {
        Vector<3> _ret;
        divide();
        _overwrite_vector<4, 3>(*this, _ret);
        return _ret;
    }
    inline Vector<3> Homogeneous::toCartesian() const
    {
        Homogeneous _retH = *this;
        return _retH.toCartesian();
    }
    inline Homogeneous& Homogeneous::apply(const TransformMixer3D& _mixer)
    {
        this->Vector<4>::operator=(lightroom::Matrix<4>(_mixer) * (*this));
        return *this;
    }

    template <size_t _SRC_SIZE, size_t _DST_SIZE>
    inline static void Homogeneous::_overwrite_vector(
        const Vector<_SRC_SIZE>& _source, Vector<_DST_SIZE>& _destination)
    {
        for (size_t i = 0; i < _SRC_SIZE && i < _DST_SIZE; i++)
        {
            _destination[i] = _source[i];
        }
    }
}