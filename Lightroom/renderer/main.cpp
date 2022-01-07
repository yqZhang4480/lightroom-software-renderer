#include "lrmath.hpp"

int main(int argc, char* argv[])
{
	lightroom::Homogeneous<4> a{ lightroom::Vector<3>{ 1,2,3 } };
	lightroom::Matrix<4> m;
	a = m * a;
	return 0;
}