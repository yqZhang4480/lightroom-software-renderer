#include "pch.h"
#include "../src/shading.hpp"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace lightroom;

namespace Microsoft
{
    namespace VisualStudio
    {
        namespace CppUnitTestFramework
        {
            template<> inline std::wstring ToString<Vector<3>>(const Vector<3>& t)
            {
                std::wstringstream wss;
                wss << "[" << t[0] << "," << t[1] << "," << t[2] << "]";
                return wss.str();
            }
            template<> inline std::wstring ToString<Vector<4>>(const Vector<4>& t)
            {
                std::wstringstream wss;
                wss << "[" << t[0] << "," << t[1] << "," << t[2] << "," << t[3] << "]";
                return wss.str();
            }
            template<> inline std::wstring ToString<Homogeneous<4>>(const Homogeneous<4>& t)
            {
                std::wstringstream wss;
                wss << "[" << t[0] << "," << t[1] << "," << t[2] << "," << t[3] << "]";
                return wss.str();
            }
            template<> inline std::wstring ToString<Matrix<4>>(const Matrix<4>& t)
            {
                std::wstringstream wss;
                wss << "[" <<
                    ToString(Vector<4>(t.row(0))) << "," <<
                    ToString(Vector<4>(t.row(1))) << "," <<
                    ToString(Vector<4>(t.row(2))) << "," <<
                    ToString(Vector<4>(t.row(3))) << "]";
                return wss.str();
            }
        }
    }
}

namespace ToString测试
{
    TEST_CLASS(TC)
    {
        TEST_METHOD(Vector_3)
        {
            auto v = Vector<3>(1, 2, 3);
            Assert::AreEqual(std::wstring(L"[1,2,3]"), ToString(v));
        }
        TEST_METHOD(Vector_4)
        {
            auto v = Vector<4>(1, 2, 3, 4);
            Assert::AreEqual(std::wstring(L"[1,2,3,4]"), ToString(v));
        }
        TEST_METHOD(Matrix_4)
        {
            auto m = Matrix<4>();
            m << 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6;
            Assert::AreEqual(std::wstring(L"[[1,2,3,4],[5,6,7,8],[9,0,1,2],[3,4,5,6]]"), ToString(m));
        }
    };
}

namespace 功能接口测试
{
    TEST_CLASS(A_齐次坐标)
    {
    public:
        TEST_METHOD(A_由VectorN_1构造)
        {
            auto h = Homogeneous<4>(Vector<3>(1, 2, 3));
            Assert::AreEqual(1.0, h[0]);
            Assert::AreEqual(2.0, h[1]);
            Assert::AreEqual(3.0, h[2]);
            Assert::AreEqual(1.0, h[3]);
        }
        TEST_METHOD(B_由VectorN构造)
        {
            auto h = Homogeneous<4>(Vector<4>(1, 2, 3, 4));
            Assert::AreEqual(1.0, h[0]);
            Assert::AreEqual(2.0, h[1]);
            Assert::AreEqual(3.0, h[2]);
            Assert::AreEqual(4.0, h[3]);
        }
        TEST_METHOD(C_齐次坐标到VectorN_1)
        {
            auto v = Vector<3>(1, 2, 3);
            auto h1 = Homogeneous<4>(v);
            Assert::AreEqual(v, h1.toOrdinary());

            auto v2 = Vector<4>(3, 6, 9, 3);
            auto h2 = Homogeneous<4>(v2);
            Assert::AreEqual(v, h2.toOrdinary());

            auto v0 = Vector<4>(1, 2, 3, 0);
            auto h3 = Homogeneous<4>(v0);
            Assert::AreEqual(v, h3.toOrdinary());

        }
        TEST_METHOD(D_赋值运算符)
        {
            auto h = Homogeneous<4>(Vector<3>{ 1, 2, 3 });
            auto _h = Homogeneous<4>(Vector<4>{ 4, 5, 6, 7 });

            h = _h;
            Assert::AreEqual(_h, h);
        }
    };

    TEST_CLASS(B_3D变换叠加器)
    {
        TEST_METHOD(A_构造函数与类型转换)
        {
            auto tm = TransformMixer3D();
            Matrix<4> expected;
            expected <<
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;

            Assert::AreEqual(expected, tm.getTransformMatrix());
        }
        TEST_METHOD(B_平移变换)
        {
            auto hv = Homogeneous<4>(Vector<4>{ 1,2,3,1 });
            auto tm = TransformMixer3D();
            hv = tm.translate(3, -2, 1).getTransformMatrix() * hv;
            
            Matrix<4> expected;
            expected <<
                1, 0, 0, 3,
                0, 1, 0, -2,
                0, 0, 1, 1,
                0, 0, 0, 1;
            
            Assert::AreEqual(expected, tm.getTransformMatrix());
            Assert::AreEqual(Vector<3>{ 4,0,4 }, hv.toOrdinary());
        }
        TEST_METHOD(C_缩放变换)
        {
            auto hv = Homogeneous<4>(Vector<4>{ 1, 2, 3, 1 });
            auto tm = TransformMixer3D();
            hv = tm.scale(3, -2, 1).getTransformMatrix() * hv;

            Matrix<4> expected;
            expected <<
                3, 0, 0, 0,
                0, -2, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;

            Assert::AreEqual(expected, tm.getTransformMatrix());
            Assert::AreEqual(Vector<3>{ 3, -4, 3 }, hv.toOrdinary());
        }
        TEST_METHOD(D_旋转变换)
        {
            auto hv = Homogeneous<4>(Vector<4>{ 1,2,3,1 });
            auto tm = TransformMixer3D(); 
            hv = tm.rotate(AngleOfDegrees[90], 0, 0)
                .getTransformMatrix() * hv;

            Matrix<4> expected;
            expected <<
                3, 0, 0, 0,
                0, -2, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;

            //Assert::AreEqual(expected, tm.getTransformMatrix());
            Assert::AreEqual(Vector<3>{ 1,-3,2 }, hv.toOrdinary());
        }
        TEST_METHOD(Z_赋值运算符)
        {

        }
    };
};