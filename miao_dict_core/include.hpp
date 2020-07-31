#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>

#include <json/json.h>

#if _HAS_CXX20 // TODO: 使用更通用的方法判断标准版本。
#define __stdge20 1
#else
#define __stdge17 1
#endif

namespace miao::core
{
	using id_t = unsigned long long;
	using uint_t = unsigned long long;

	/// <summary>
	/// 可序列化类型基类。一个类型如果是可序列化的，它应该是 serializale 的子类。
	/// </summary>
	class serializable_base
	{
	public:
		virtual std::string to_string() const = 0;
		virtual void from_string(std::string_view str) = 0;
	};
	/// <summary>
	/// 反序列化时 JSON 解析错误。
	/// </summary>
	class parse_error : public std::exception { using std::exception::exception; };
	/// <summary>
	/// 反序列化时存在数据无法正确反序列化。
	/// </summary>
	class deserialize_error : public std::exception { using std::exception::exception; };
#if __stdge20
	template<typename T>
	concept serializable = requires(T a)
	{
		std::is_same_v<std::string, decltype(a.to_string())>;
		std::is_same_v<void, decltype(a.from_string(std::string()))>;
	};
	static_assert(serializable<serializable_base>);
#endif
}