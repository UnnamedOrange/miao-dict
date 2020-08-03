#pragma once

#include "include.hpp"
#include "item.hpp"

namespace miao::core
{
	class library : public serializable_base
	{
	public:
		// 版本标记
		static constexpr int latest_ver_tag = 1;
		int ver_tag;

		// 库信息
		id_t id;
		std::u32string tag;
		std::u32string lang;

		// dict
		std::unordered_map<id_t, item> items;
		std::vector<raw_item> raw_items;

		// raw
		std::vector<std::u32string> passages;

		// pronunciations
		// TODO

	public:
		[[nodiscard]] virtual Json::Value to_json() const override
		{
			Json::Value root;
			root["id"] = id;
			root["tag"] = utf_conv<char32_t, char>::convert(tag);
			root["lang"] = utf_conv<char32_t, char>::convert(lang);
			return root;
		}
		virtual void from_json(const Json::Value& value) override
		{
			ver_tag = 0;
			try
			{
				Json::value_assign(id, value["id"]);
				Json::value_assign(tag, value["tag"]);
				Json::value_assign(tag, value["lang"]);
				ver_tag = 1;
			}
			catch (...)
			{
				if (!ver_tag)
					throw deserialize_error("fail to translate the string into library.");
			}
		}
	};
}