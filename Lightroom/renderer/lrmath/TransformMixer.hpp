#pragma once
#include "lrmath_utility.hpp"

namespace lightroom
{
    // TODO: use template.
    class TransformMixer3D : protected Matrix<4>
    {
    public:
        using lightroom::Matrix<4>::operator=;
        TransformMixer3D() : Matrix()
        {
            *this = this->Identity();
        }
        operator const lightroom::Matrix<4>&() const
        {
            return *static_cast<const lightroom::Matrix<4>*>(this);
        }
        const lightroom::Matrix<4>& getTransformMatrix() const
        {
            return *this;
        }

        inline TransformMixer3D& scale(Float _factorX, Float _factorY, Float  _factorZ)
        {
            lightroom::Matrix<4> _lpm;
            _lpm <<
                _factorX, 0, 0, 0,
                0, _factorY, 0, 0,
                0, 0, _factorZ, 0,
                0, 0, 0, 1;
            return apply(_lpm);
        }
        inline TransformMixer3D& rotate(Angle _x, Angle _y, Angle _z)
        {
            using lightroom::sin;
            using lightroom::cos;
            lightroom::Matrix<4> _rx, _ry, _rz;
            _rx <<
                1, 0, 0, 0,
                0, cos(_x), -sin(_x), 0,
                0, sin(_x), cos(_x), 0,
                0, 0, 0, 1;
            _ry <<
                cos(_y), 0, sin(_y), 0,
                0, 1, 0, 0,
                -sin(_y), 0, cos(_y), 0,
                0, 0, 0, 1;
            _rz <<
                cos(_z), -sin(_z), 0, 0,
                sin(_z), cos(_z), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
            return apply(_rz * _ry * _rx);

        }
        inline TransformMixer3D& translate(Float _differenceX, Float _differenceY, Float  _differenceZ)
        {
            lightroom::Matrix<4> _lpm;
            _lpm <<
                1, 0, 0, _differenceX,
                0, 1, 0, _differenceY,
                0, 0, 1, _differenceZ,
                0, 0, 0, 1;
            return apply(_lpm);
        }
        inline TransformMixer3D& changeBase(
            const Vector<3>& _originPoint,
            const Vector<3>& _axisU,
            const Vector<3>& _axisV,
            const Vector<3>& _axisW
            )
        {
            lightroom::Matrix<4> _rotate;
            _rotate <<
                _axisU[0], _axisU[1], _axisU[2], 0,
                _axisV[0], _axisV[1], _axisV[2], 0,
                _axisW[0], _axisW[1], _axisW[2], 0,
                0, 0, 0, 1;
            return (*this).translate(-_originPoint[0], -_originPoint[1], -_originPoint[2])
                          .apply(_rotate);
        }

        inline TransformMixer3D& apply(const lightroom::Matrix<4>& _matrix)
        {
            *this = _matrix * (*this);
            return *this;
        }
    };
    class TransformMixer2D : protected Matrix<3>
    {
    public:
        using lightroom::Matrix<3>::operator=;
        TransformMixer2D() : Matrix()
        {
            *this = this->Identity();
        }
        operator const lightroom::Matrix<3>& () const
        {
            return *static_cast<const lightroom::Matrix<3>*>(this);
        }
        const lightroom::Matrix<3>& getTransformMatrix() const
        {
            return *this;
        }

        inline TransformMixer2D& scale(Float _factorX, Float _factorY)
        {
            lightroom::Matrix<3> _lpm;
            _lpm <<
                _factorX, 0, 0,
                0, _factorY, 0,
                0, 0, 1;
            return apply(_lpm);
        }
        inline TransformMixer2D& rotate(Angle _z)
        {
            using lightroom::sin;
            using lightroom::cos;
            lightroom::Matrix<3> _lpm;
                cos(_z), -sin(_z), 0,
                sin(_z), cos(_z), 0,
                0, 0, 1;
            return apply(_lpm);
        }
        inline TransformMixer2D& translate(Float _differenceX, Float _differenceY)
        {
            lightroom::Matrix<3> _lpm;
            _lpm <<
                1, 0, _differenceX,
                0, 1, _differenceY,
                0, 0, 1;
            return apply(_lpm);
        }
        inline TransformMixer2D& changeBase(
            const Vector<2>& _originPoint,
            const Vector<2>& _axisU,
            const Vector<2>& _axisV
        )
        {
            lightroom::Matrix<3> _rotate;
            _rotate <<
                _axisU[0], _axisU[1], 0,
                _axisV[0], _axisV[1], 0,
                0, 0, 1;
            return (*this).translate(-_originPoint[0], -_originPoint[1]).apply(_rotate);
        }

        inline TransformMixer2D& apply(const lightroom::Matrix<3>& _matrix)
        {
            *this = _matrix * (*this);
            return *this;
        }
    };
}