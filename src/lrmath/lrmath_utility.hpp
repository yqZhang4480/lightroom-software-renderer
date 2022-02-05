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

    class TransformMixer3D;
    class Homogeneous;

    class TransformMixer3D : private Matrix<4>
    {
        template<typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
        friend class Eigen::Matrix;
        friend class Homogeneous;
    public:
        // ���첻�����κα任�����Ķ���
        inline TransformMixer3D();

        // ����
        inline TransformMixer3D& scale(Float _factorX, Float _factorY, Float  _factorZ)&;
        inline TransformMixer3D scale(Float _factorX, Float _factorY, Float  _factorZ)&&;

        // ��ת
        inline TransformMixer3D& rotate(Angle _angleX, Angle _angleY, Angle _angleZ)&;
        inline TransformMixer3D rotate(Angle _angleX, Angle _angleY, Angle _angleZ)&&;

        // ƽ��
        inline TransformMixer3D& translate(Float _differenceX, Float _differenceY, Float  _differenceZ)&;
        inline TransformMixer3D translate(Float _differenceX, Float _differenceY, Float  _differenceZ)&&;

        // �任����ϵ
        inline TransformMixer3D& changeBase(
            const Vector<3>& _originPoint, Vector<3> _axisU, Vector<3> _axisV, Vector<3> _axisW)&;
        inline TransformMixer3D changeBase(
            const Vector<3>& _originPoint, Vector<3> _axisU, Vector<3> _axisV, Vector<3> _axisW)&&;

        // �����任
        inline TransformMixer3D& apply(const lightroom::Matrix<4>& _matrix)&;
        inline TransformMixer3D apply(const lightroom::Matrix<4>& _matrix)&&;

    private:
        inline lightroom::Matrix<4> _createScaleMatrix(Float, Float, Float) const;
        inline lightroom::Matrix<4> _createRotateMatrix(Angle, Angle, Angle) const;
        inline lightroom::Matrix<4> _createTranslateMatrix(Float, Float, Float) const;
    };
};
#endif // !_LRMATH_UTILITY_