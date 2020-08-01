#pragma once

#include "include.hpp"

namespace miao::core
{
	class item
	{
	public:
		// 基础数据
		id_t id{};
		std::u32string origin;
		std::vector<std::u32string> variants;
		std::vector<std::u32string> notations;
		std::vector<std::tuple<id_t, id_t, std::u32string, std::u32string>> translations; // (id, lib_id, tag, meaning)
		std::vector<id_t> pronunciations;
		std::vector<std::tuple<id_t, id_t>> sentences;

		// 常驻显示
		uint_t showing_time{};
		uint_t n_skips{};

		// 轮巡显示
		uint_t n_flick{};
		uint_t n_pause{};
		uint_t n_pronounce{};

		// 查询
		uint_t n_query{};
	};

	class raw_item final : public serializable_base
	{
	public:
		std::u32string origin;
		uint_t frequency{};

	public:
		[[nodiscard]] virtual std::u8string to_string() const override
		{
			Json::Value root;
			root["frequency"] = frequency;
			root["origin"] = utf_conv<char32_t, char>::convert(origin);
			return Json::write(root);
		}
		virtual void from_string(std::u8string_view str) override
		{
			Json::Value root = Json::read(str);

			try
			{
				frequency = root["frequency"].asUInt64();
				origin = utf_conv<char, char32_t>::convert(root["origin"].asCString());
			}
			catch (...)
			{
				throw deserialize_error("fail to translate the string into raw_item.");
			}
		}
	};
}