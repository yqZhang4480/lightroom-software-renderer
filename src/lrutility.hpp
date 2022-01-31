#pragma once
#ifndef _LRUTILITY_
#define _LRUTILITY_

#pragma region INCLUDE
#define WINVER 0x0500
#define _WIN32_WINNT 0x0A00
#ifndef _mutex_
#define _mutex_
#include <mutex>
#endif // !_mutex_

#ifndef _minmax_H_
#define _minmax_H_
#include <minmax.h>
#endif // !_minmax_H_

#ifndef _eigen_
#define _eigen_
#include "Eigen/Dense"
#endif _eigen_

#ifndef _graphics_H_
#define _graphics_H_
#include "easyx/graphics.h"
#endif // !_graphics_H_

#ifndef _vector_
#define _vector_
#include <vector>
#endif // !_vector_

#ifndef _memory_
#define _memory_
#include <memory>
#endif // !_memory_

#ifndef _stack_
#define _stack_
#include <stack>
#endif // !_stack_

#ifndef _array_
#define _array_
#include <array>
#endif // !_array_

#ifndef _tuple_
#define _tuple_
#include <tuple>
#endif // !_tuple_

#ifndef _list_
#define _list_
#include <list>
#endif // !_list_

#ifndef _unordered_map_
#define _unordered_map_
#include <unordered_map>
#endif // !_unordered_map_

#ifndef _ShellScalingApi_H_
#define _ShellScalingApi_H_
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#endif // !_ShellScalingApi_H_

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#pragma endregion

namespace lightroom
{
    using Float = double;

    using Angle = Float;
    
    inline Float sin(Angle _angle)
    {
        return ::sin(_angle);
    }
    inline Float cos(Angle _angle)
    {
        return ::cos(_angle);
    }
}

#endif _LRUTILITY_