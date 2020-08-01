#pragma once

#include "include.hpp"

namespace miao
{
	/// <summary>
	/// 在 UTF 编码中进行转换。只有部分特化模板具有实现。
	/// </summary>
	/// <typeparam name="src_t">源的字符类型。</typeparam>
	/// <typeparam name="des_t">目标字符类型。如果是宽字符，使用小端编码。</typeparam>
	template <typename src_t, typename des_t>
	class utf_conv
	{
	public:
		static std::basic_string<des_t> convert(std::basic_string_view<src_t> src)
		{
			static_assert(false, "src_t and des_t is invalid.");
		}
	};
	/// <summary>
	/// 从 UTF-8 转换到 UTF-32（LE）。
	/// </summary>
	/// <typeparam name="char_or_char8_t">char 或者 char8_t（C++20）。</typeparam>
	template <typename char_or_char8_t>
	class utf_conv<char_or_char8_t, char32_t>
	{
	public:
		static std::u32string convert(std::basic_string_view<char_or_char8_t> src)
#if __stdge20
		requires std::is_same_v<char_or_char8_t, char> || std::is_same_v<char_or_char8_t, char8_t>
#endif
		{
#if !__stdge20
			static_assert(std::is_same_v<char_or_char8_t, char>, "src_t and des_t is invalid.");
#endif

			return std::u32string();
		}
	};
	/// <summary>
	/// 从 UTF-32（LE） 转换到 UTF-8。
	/// </summary>
	/// <typeparam name="char_or_char8_t">char 或者 char8_t（C++20）。</typeparam>
	template <typename char_or_char8_t>
	class utf_conv<char32_t, char_or_char8_t>
	{
	public:
		static std::basic_string<char_or_char8_t> convert(std::u32string_view src)
#if __stdge20
			requires std::is_same_v<char_or_char8_t, char> || std::is_same_v<char_or_char8_t, char8_t>
#endif
		{
#if !__stdge20
			static_assert(std::is_same_v<char_or_char8_t, char>, "src_t and des_t is invalid.");
#endif

			return std::basic_string<char_or_char8_t>();
		}
	};
}