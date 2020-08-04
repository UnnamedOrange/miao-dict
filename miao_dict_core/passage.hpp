#pragma once

#include "include.hpp"

namespace miao::core
{
	class passage final : public serializable_base
	{
	public:
		// 版本标记
		static constexpr int latest_ver_tag = 2;
		int ver_tag = latest_ver_tag;

		// 数据域
		id_t id{};
		std::u32string content;

		// 版本 2
		std::u32string abstract;

	public:
		[[nodiscard]] virtual Json::Value to_json() const override
		{
			Json::Value root;
			root["content"] = utf_conv<char32_t, char>::convert(content);
			root["id"] = id;
			root["abstract"] = utf_conv<char32_t, char>::convert(abstract);
			return root;
		}
		virtual void from_json(const Json::Value& value) override
		{
			ver_tag = 0;
			try
			{
				Json::value_assign(content, value["content"]);
				id = value["id"].asUInt64();
				ver_tag = 1;

				Json::value_assign(abstract, value["abstract"]);
				ver_tag = 2;
			}
			catch (...)
			{
				if (!ver_tag)
					throw deserialize_error("fail to translate the json into passage.");
			}
		}
	};
}