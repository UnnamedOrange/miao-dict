#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <fstream>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <variant>
#include <optional>

#include <json/json.h>

#include "cppver.hpp"
#include "utf_conv.hpp"

namespace miao::core
{
	/// <summary>
	/// 反序列化时 JSON 解析错误。
	/// </summary>
	class parse_error : public std::runtime_error { using std::runtime_error::runtime_error; };
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
			builder.settings_["emitUTF8"] = true;
			std::unique_ptr<Json::StreamWriter> const writer(builder.newStreamWriter());
			writer->write(v, &ss);
		}
		return std::u8string(reinterpret_cast<const char8_t*>(ss.str().c_str()));
	}

	using value_t = std::variant<std::monostate, long long, unsigned long long, double, std::u32string, bool, std::vector<Json::Value>, Json::Value>;
	inline value_t value_cast(const Value& v)
	{
		switch (v.type())
		{
		case nullValue:
			return std::monostate();
		case intValue:
			return v.asInt64();
		case uintValue:
			return v.asUInt64();
		case realValue:
			return v.asDouble();
		case stringValue:
			return miao::utf_conv<char, char32_t>::convert(v.asCString());
		case booleanValue:
			return v.asBool();
		case arrayValue:
		{
			std::vector<Json::Value> ret(v.size());
			for (size_t i = 0; i < ret.size(); i++)
				ret[i] = v[static_cast<Json::ArrayIndex>(i)];
			return ret;
		}
		case objectValue:
			return v;
		}
		throw std::runtime_error("v.type() is invalid.");
	}
	template<typename T>
	inline void value_assign(T& t, const value_t& v)
	{
		t = std::get<T>(v);
	}
	template<typename T>
	inline void value_assign(T& t, const Json::Value& v)
	{
		value_assign(t, value_cast(v));
	}
}

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
		[[nodiscard]] virtual Json::Value to_json() const = 0;
		[[nodiscard]] std::u8string to_string() const
		{
			return Json::write(to_json());
		}
		virtual void from_json(const Json::Value& value) = 0;
		virtual void from_string(std::u8string_view str)
		{
			from_json(Json::read(str));
		}
		/// <summary>
		/// 将整个文件内容作为参数调用 from_string。如果文件不存在，将抛出 std::runtime_error。
		/// </summary>
		/// <param name="filename">文件名。</param>
		void from_file(std::filesystem::path filename)
		{
			filename.make_preferred();
			if (!std::filesystem::exists(filename))
				throw std::runtime_error("file doesn't exists.");

			std::ifstream fs(filename);
			fs.seekg(0, std::ios::end);
			size_t len = fs.tellg();
			fs.seekg(0, std::ios::beg);
			std::vector<char8_t> buf(len + 1);
			fs.read(reinterpret_cast<char*>(buf.data()), len);
			from_string(buf.data());
		}
		void to_file(std::filesystem::path filename) const
		{
			filename.make_preferred();
			std::ofstream fs(filename);
			std::u8string str = to_string();
			fs.write(reinterpret_cast<char*>(str.data()), str.length());
		}
	};
	/// <summary>
	/// 反序列化时存在数据无法正确反序列化。
	/// </summary>
	class deserialize_error : public std::runtime_error { using std::runtime_error::runtime_error; };

#if __stdge20
	template<typename T>
	concept serializable = requires(T a)
	{
		std::is_same_v<Json::Value, decltype(a.to_json())>;
		std::is_same_v<void, decltype(a.from_json(Json::Value()))>;
		std::is_same_v<std::u8string, decltype(a.to_string())>;
		std::is_same_v<void, decltype(a.from_string(std::u8string()))>;
	};
	static_assert(serializable<serializable_base>);
#endif
}