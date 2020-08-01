#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <array>
#include <stdexcept>
#include <type_traits>

#include <json/json.h>

#include "cppver.hpp"
#include "utf_conv.hpp"

namespace miao::core
{
	using id_t = unsigned long long;
	using uint_t = unsigned long long;
	using dbcs_string = std::string;
	using dbcs_string_view = std::string_view;

	/// <summary>
	/// 可序列化类型基类。一个类型如果是可序列化的，它应该是 serializale 的子类。
	/// </summary>
	class serializable_base
	{
	public:
		virtual std::u8string to_string() const = 0;
		virtual void from_string(std::u8string_view str) = 0;
		/// <summary>
		/// 将整个文件内容作为参数调用 from_string。
		/// </summary>
		/// <param name="filename">文件名。</param>
		void from_file(dbcs_string_view filename)
		{
			std::fstream fs(filename.data(), std::ios::in | std::ios::binary);
			fs.is_open();
			fs.seekg(0, std::ios::end);
			size_t len = fs.tellg();
			fs.seekg(0, std::ios::beg);
			std::vector<char8_t> buf(len + 1);
			fs.read(reinterpret_cast<char*>(buf.data()), len);
			from_string(buf.data());
		}
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
		std::is_same_v<std::u8string, decltype(a.to_string())>;
		std::is_same_v<void, decltype(a.from_string(std::u8string()))>;
	};
	static_assert(serializable<serializable_base>);
#endif
}