#pragma once
#include "lrmath_utility.hpp"

namespace lightroom
{
    inline TransformMixer3D::TransformMixer3D() : Matrix(lightroom::Matrix<4>::Identity()) {}

    inline TransformMixer3D& TransformMixer3D::scale(Float _factorX, Float _factorY, Float  _factorZ)&
    {
        return apply(_createScaleMatrix(_factorX, _factorY, _factorZ));
    }
    inline TransformMixer3D TransformMixer3D::scale(Float _factorX, Float _factorY, Float  _factorZ)&&
    {
        return apply(_createScaleMatrix(_factorX, _factorY, _factorZ));
    }

    inline TransformMixer3D& TransformMixer3D::rotate(Angle _angleX, Angle _angleY, Angle _angleZ)&
    {
        return apply(_createRotateMatrix(_angleX, _angleY, _angleZ));
    }
    inline TransformMixer3D TransformMixer3D::rotate(Angle _angleX, Angle _angleY, Angle _angleZ)&&
    {
        return apply(_createRotateMatrix(_angleX, _angleY, _angleZ));
    }

    inline TransformMixer3D& TransformMixer3D::translate(Float _differenceX, Float _differenceY, Float  _differenceZ)&
    {
        return apply(_createTranslateMatrix(_differenceX, _differenceY, _differenceZ));
    }
    inline TransformMixer3D TransformMixer3D::translate(Float _differenceX, Float _differenceY, Float  _differenceZ)&&
    {
        return apply(_createTranslateMatrix(_differenceX, _differenceY, _differenceZ));
    }

    inline TransformMixer3D& TransformMixer3D::changeBase(
        const Vector<3>& _originPoint, Vector<3> _axisU, Vector<3> _axisV, Vector<3> _axisW)&
    {
        _axisU.normalize();
        _axisV.normalize();
        _axisW.normalize();
        lightroom::Matrix<4> _rotate;
        _rotate <<
            _axisU[0], _axisU[1], _axisU[2], 0,
            _axisV[0], _axisV[1], _axisV[2], 0,
            _axisW[0], _axisW[1], _axisW[2], 0,
            0, 0, 0, 1;
        return (*this).translate(-_originPoint[0], -_originPoint[1], -_originPoint[2])
            .apply(_rotate);
    }
    inline TransformMixer3D TransformMixer3D::changeBase(
        const Vector<3>& _originPoint, Vector<3> _axisU, Vector<3> _axisV, Vector<3> _axisW)&&
    {
        _axisU.normalize();
        _axisV.normalize();
        _axisW.normalize();
        lightroom::Matrix<4> _rotate;
        _rotate <<
            _axisU[0], _axisU[1], _axisU[2], 0,
            _axisV[0], _axisV[1], _axisV[2], 0,
            _axisW[0], _axisW[1], _axisW[2], 0,
            0, 0, 0, 1;
        return (*this).translate(-_originPoint[0], -_originPoint[1], -_originPoint[2])
            .apply(_rotate);
    }

    inline TransformMixer3D& TransformMixer3D::apply(const lightroom::Matrix<4>& _matrix)&
    {
        this->lightroom::Matrix<4>::operator=(_matrix * (*this));
        return *this;
    }
    inline TransformMixer3D TransformMixer3D::apply(const lightroom::Matrix<4>& _matrix)&&
    {
        this->lightroom::Matrix<4>::operator=(_matrix * (*this));
        return *this;
    }

    inline lightroom::Matrix<4> TransformMixer3D::_createScaleMatrix(Float _factorX, Float _factorY, Float _factorZ) const
    {
        lightroom::Matrix<4> _ret;
        _ret <<
            _factorX, 0, 0, 0,
            0, _factorY, 0, 0,
            0, 0, _factorZ, 0,
            0, 0, 0, 1;
        return _ret;
    }
    inline lightroom::Matrix<4> TransformMixer3D::_createTranslateMatrix(Float _differenceX, Float _differenceY, Float _differenceZ) const
    {
        lightroom::Matrix<4> _ret;
        _ret <<
            1, 0, 0, _differenceX,
            0, 1, 0, _differenceY,
            0, 0, 1, _differenceZ,
            0, 0, 0, 1;
        return _ret;
    }
    inline lightroom::Matrix<4> TransformMixer3D::_createRotateMatrix(Angle _x, Angle _y, Angle _z) const
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
        return _rz * _ry * _rx;
    }
}