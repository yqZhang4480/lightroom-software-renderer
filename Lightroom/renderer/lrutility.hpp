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
#include <graphics.h>
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

    using Angle = uint16_t;
    constexpr const Angle AngleAsDegree[360] =
    {
        0,
        182,  364,  546,  728,  910,  1092, 1274, 1456, 1638, 1820,
        2002, 2184, 2366, 2548, 2730, 2912, 3094, 3276, 3458, 3640,
        3822, 4004, 4187, 4369, 4551, 4733, 4915, 5097, 5279, 5461,

        5643, 5825, 6007, 6189, 6371,  6553,  6735,  6917,  7099,  7281,
        7463, 7645, 7827, 8009, 8192,  8374,  8556,  8738,  8920,  9102,
        9284, 9466, 9648, 9830, 10012, 10194, 10376, 10558, 10740, 10922,

        11104, 11286, 11468, 11650, 11832, 12014, 12196, 12379, 12561, 12743,
        12925, 13107, 13289, 13471, 13653, 13835, 14017, 14199, 14381, 14563,
        14745, 14927, 15109, 15291, 15473, 15655, 15837, 16019, 16201, 16384,

        16566, 16748, 16930, 17112, 17294, 17476, 17658, 17840, 18022, 18204,
        18386, 18568, 18750, 18932, 19114, 19296, 19478, 19660, 19842, 20024,
        20206, 20388, 20571, 20753, 20935, 21117, 21299, 21481, 21663, 21845,

        22027, 22209, 22391, 22573, 22755, 22937, 23119, 23301, 23483, 23665,
        23847, 24029, 24211, 24393, 24576, 24758, 24940, 25122, 25304, 25486,
        25668, 25850, 26032, 26214, 26396, 26578, 26760, 26942, 27124, 27306,

        27488, 27670, 27852, 28034, 28216, 28398, 28580, 28763, 28945, 29127,
        29309, 29491, 29673, 29855, 30037, 30219, 30401, 30583, 30765, 30947,
        31129, 31311, 31493, 31675, 31857, 32039, 32221, 32403, 32585, 32768,

        32950, 33132, 33314, 33496, 33678, 33860, 34042, 34224, 34406, 34588,
        34770, 34952, 35134, 35316, 35498, 35680, 35862, 36044, 36226, 36408,
        36590, 36772, 36955, 37137, 37319, 37501, 37683, 37865, 38047, 38229,

        38411, 38593, 38775, 38957, 39139, 39321, 39503, 39685, 39867, 40049,
        40231, 40413, 40595, 40777, 40960, 41142, 41324, 41506, 41688, 41870,
        42052, 42234, 42416, 42598, 42780, 42962, 43144, 43326, 43508, 43690,

        43872, 44054, 44236, 44418, 44600, 44782, 44964, 45147, 45329, 45511,
        45693, 45875, 46057, 46239, 46421, 46603, 46785, 46967, 47149, 47331,
        47513, 47695, 47877, 48059, 48241, 48423, 48605, 48787, 48969, 49152,

        49334, 49516, 49698, 49880, 50062, 50244, 50426, 50608, 50790, 50972,
        51154, 51336, 51518, 51700, 51882, 52064, 52246, 52428, 52610, 52792,
        52974, 53156, 53339, 53521, 53703, 53885, 54067, 54249, 54431, 54613,

        54795, 54977, 55159, 55341, 55523, 55705, 55887, 56069, 56251, 56433,
        56615, 56797, 56979, 57161, 57344, 57526, 57708, 57890, 58072, 58254,
        58436, 58618, 58800, 58982, 59164, 59346, 59528, 59710, 59892, 60074,

        60256, 60438, 60620, 60802, 60984, 61166, 61348, 61531, 61713, 61895,
        62077, 62259, 62441, 62623, 62805, 62987, 63169, 63351, 63533, 63715,
        63897, 64079, 64261, 64443, 64625, 64807, 64989, 65171, 65353
    };
    namespace _internal
    {
        constexpr const Float sinTable_16[65536] =
        {
        #include "sinetable_16.csv"
        };
    };
    constexpr inline Float sin(Angle _angle)
    {
        return _internal::sinTable_16[_angle];
    }
    constexpr inline Float cos(Angle _angle)
    {
        return sin(_angle + 16384);
    }
}

#endif _LRUTILITY_