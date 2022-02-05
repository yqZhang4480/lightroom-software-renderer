#pragma once
#include "lrmath_utility.hpp"

namespace lightroom
{
    class Homogeneous final : public Vector<4>
    {
    public:
        // �ɵѿ������깹���������
        inline Homogeneous(const Vector<3>& _cartesianCoordinate);

        // �Ը���������w
        inline void divide();

        // ����������õѿ�������
        inline Vector<3> toCartesian();
        inline Vector<3> toCartesian() const;
        
        // Ӧ�ñ任
        inline Homogeneous& apply(const TransformMixer3D& _mixer);

    private:
        template <size_t _SRC_SIZE, size_t _DST_SIZE>
        inline static void _overwrite_vector(
            const Vector<_SRC_SIZE>& _source, Vector<_DST_SIZE>& _destination);
    };

    inline Homogeneous::Homogeneous(const Vector<3>& _cartesianCoordinate)
    {
        _overwrite_vector<3, 4>(_cartesianCoordinate, *this);
        (*this)[3] = 1;
    }

    inline void Homogeneous::divide()
    {
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
        Vector<3> _ret;
        for (size_t _i = 0; _i < 3; _i++)
        {
            _ret[_i] = (*this)[_i] / (*this)[3];
        }
        return _ret;
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