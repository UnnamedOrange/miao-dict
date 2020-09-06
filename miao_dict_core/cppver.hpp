#pragma once

#if _HAS_CXX20 // TODO: 使用更通用的方法判断标准版本。
#define __stdge20 1
#else
#define __stdge17 1
#endif

#if _MSVC_LANG
#define __windows 1
#else
#define __unix 1
#endif