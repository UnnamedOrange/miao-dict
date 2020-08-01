#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <filesystem>

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
		[[nodiscard]] virtual std::u8string to_string() const = 0;
		virtual void from_string(std::u8string_view str) = 0;
		/// <summary>
		/// 将整个文件内容作为参数调用 from_string。
		/// </summary>
		/// <param name="filename">文件名。</param>
		void from_file(dbcs_string_view filename)
		{
			if (!std::filesystem::exists(filename))
				throw std::runtime_error("file doesn't exists.");

			std::fstream fs(filename.data());
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
	class parse_error : public std::runtime_error { using std::runtime_error::runtime_error; };
	/// <summary>
	/// 反序列化时存在数据无法正确反序列化。
	/// </summary>
	class deserialize_error : public std::runtime_error { using std::runtime_error::runtime_error; };

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

namespace Json
{
	/// <summary>
	/// 将字符串（std::u8string_view）转换为 Json::Value。
	/// </summary>
	/// <param name="str">源字符串。</param>
	/// <returns>转换后的 Json::Value。</returns>
	[[nodiscard]] inline Value read(std::u8string_view str)
	{
		Value ret;
		CharReaderBuilder builder;
		std::unique_ptr<CharReader> const reader(builder.newCharReader());
		if (Json::String error;
			!reader->parse(reinterpret_cast<const char*>(str.data()),
				reinterpret_cast<const char*>(str.data()) + str.size(), &ret, &error))
			throw miao::core::parse_error(("fail to reader->parse." + error).c_str());
		return ret;
	}
	/// <summary>
	/// 将 Json::Value 转换为字符串（std::u8string）。
	/// </summary>
	/// <param name="v">Json::Value 类型的常值引用。</param>
	/// <returns>转换后的字符串。</returns>
	[[nodiscard]] inline std::u8string write(const Value& v)
	{
		std::ostringstream ss;
		{
			Json::StreamWriterBuilder builder;
			std::unique_ptr<Json::StreamWriter> const writer(builder.newStreamWriter());
			writer->write(v, &ss);
		}
		return std::u8string(reinterpret_cast<const char8_t*>(ss.str().c_str()));
	}
}