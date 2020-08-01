#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <tuple>
#include <stdexcept>
#include <type_traits>
#include "cppver.hpp"

#if !__stdge20
using char8_t = char;
namespace std
{
	using u8string = std::basic_string<char8_t>;
	using u8string_view = std::basic_string_view<char8_t>;
}
#endif

namespace miao
{
	/// <summary>
	/// 在 UTF 编码间进行转换。只有部分特化模板具有实现。
	/// </summary>
	/// <typeparam name="src_t">源的字符类型。</typeparam>
	/// <typeparam name="des_t">目标字符类型。如果是宽字符，使用小端编码。</typeparam>
	template <typename src_t, typename des_t>
	class utf_conv {};
	/// <summary>
	/// 在 UTF 编码间进行转换时出错。
	/// </summary>
	class utf_conv_error : public std::runtime_error { using std::runtime_error::runtime_error; };

	/// <summary>
	/// 从 UTF-8 转换到 UTF-32（LE）。
	/// </summary>
	/// <typeparam name="char_or_char8_t">char 或者 char8_t（C++20）。</typeparam>
	template <typename char_or_char8_t>
	class utf_conv<char_or_char8_t, char32_t>
	{
	private:
		static std::tuple<char32_t, size_t> convert_once(std::basic_string_view<char_or_char8_t> src)
		{
			if (src.empty())
				return { 0, 0 };

			char32_t ret{};
			size_t len{};
			std::make_unsigned_t<char_or_char8_t> b = src.front();

			if (b < 0x80) // 单字符
				return { b, 1 };
			else if (b < 0xC0 || b > 0xFD) // 非法值。
				throw utf_conv_error("fail to convert_once. invalid utf-8 char.");
			else if (b < 0xE0)
			{
				ret = b & 0x1F;
				len = 2;
			}
			else if (b < 0xF0)
			{
				ret = b & 0x0F;
				len = 3;
			}
			else if (b < 0xF8u)
			{
				ret = b & 7;
				len = 4;
			}
			else if (b < 0xFCu)
			{
				ret = b & 3;
				len = 5;
			}
			else
			{
				ret = b & 1;
				len = 6;
			}

			for (size_t i = 1; i < len; i++)
			{
				b = src[i];
				if (b < 0x80 || b > 0xBF) // 非法值。
					break;

				ret = (ret << 6) + (b & 0x3F);
			}
			return { ret, len };
		}
	public:
		[[nodiscard]] static std::u32string convert(std::basic_string_view<char_or_char8_t> src)
#if __stdge20
			requires std::is_same_v<char_or_char8_t, char> || std::is_same_v<char_or_char8_t, char8_t>
#endif
		{
#if !__stdge20
			static_assert(std::is_same_v<char_or_char8_t, char>, "src_t and des_t is invalid.");
#endif
			size_t length{};
			decltype(convert_once(src)) t;
			for (size_t i = 0; i < src.length(); i += std::get<1>(t))
			{
				t = convert_once(src.substr(i));
				length++;
			}
			std::vector<char32_t> ret(length + 1);
			length = 0;
			for (size_t i = 0; i < src.length(); i += std::get<1>(t))
			{
				t = convert_once(src.substr(i));
				ret[length++] = std::get<0>(t);
			}
			return ret.data();
		}
	};
	/// <summary>
	/// 从 UTF-32（LE） 转换到 UTF-8。
	/// </summary>
	/// <typeparam name="char_or_char8_t">char 或者 char8_t（C++20）。</typeparam>
	template <typename char_or_char8_t>
	class utf_conv<char32_t, char_or_char8_t>
	{
	private:
		static std::tuple<std::array<char_or_char8_t, 6>, size_t> convert_once(char32_t ch)
		{
			constexpr std::array<std::make_unsigned_t<char_or_char8_t>, 6> prefix
			{ 0, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
			constexpr std::array<char32_t, 6> code_up
			{ 0x80, 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000 };

			std::array<char_or_char8_t, 6> ret{};
			size_t len{};
			// 根据 UTF-32 编码范围确定对应的 UTF-8 编码字节数。
			bool valid{};
			for (size_t i = 0; i < code_up.size(); i++)
				if (ch < code_up[i])
				{
					len = i + 1;
					valid = true;
					break;
				}
			if (!valid)
				throw utf_conv_error("fail to convert_once. invalid utf-32 char.");

			for (size_t i = len - 1; i; i--)
			{
				ret[i] = static_cast<char_or_char8_t>((ch & 0x3F) | 0x80);
				ch >>= 6;
			}
			ret[0] = static_cast<char_or_char8_t>(ch | prefix[len - 1]);
			return { ret, len };
		}
	public:
		[[nodiscard]] static std::basic_string<char_or_char8_t> convert(std::u32string_view src)
#if __stdge20
			requires std::is_same_v<char_or_char8_t, char> || std::is_same_v<char_or_char8_t, char8_t>
#endif
		{
#if !__stdge20
			static_assert(std::is_same_v<char_or_char8_t, char>, "src_t and des_t is invalid.");
#endif
			size_t length{};
			decltype(convert_once(src[0])) t;
			for (size_t i = 0; i < src.length(); i++)
			{
				t = convert_once(src[i]);
				length += std::get<1>(t);
			}
			std::vector<char_or_char8_t> ret(length + 1);
			length = 0;
			for (size_t i = 0; i < src.length(); i++)
			{
				t = convert_once(src[i]);
				for (size_t j = 0; j < std::get<1>(t); j++)
					ret[length++] = std::get<0>(t)[j];
			}
			return ret.data();
		}
	};

	[[nodiscard]] inline std::u8string operator"" _u8(const char32_t* _Str, size_t _Len) {
		return utf_conv<char32_t, char8_t>::convert(std::u32string_view(_Str, _Len));
	}
}