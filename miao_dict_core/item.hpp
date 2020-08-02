#pragma once

#include "include.hpp"

namespace miao::core
{
	class item : public serializable_base
	{
	public:
		// 基础数据
		id_t id{};
		std::u32string origin;
		std::vector<std::u32string> variants;
		std::vector<std::u32string> notations;
		std::vector<std::tuple<id_t, id_t, std::u32string, std::u32string>> translations; // (id, lib_id, tag, meaning)
		std::vector<id_t> pronunciations;
		std::vector<std::tuple<id_t, id_t>> sentences; // (id, trans_id)

		// 常驻显示
		uint_t showing_time{};
		uint_t n_skips{};

		// 轮巡显示
		uint_t n_flick{};
		uint_t n_pause{};
		uint_t n_pronounce{};

		// 查询
		uint_t n_query{};

	public:
		[[nodiscard]] virtual std::u8string to_string() const override
		{
			Json::Value root;
			root["id"] = id;
			root["origin"] = utf_conv<char32_t, char>::convert(origin);

			root["variants"].resize(0);
			for (const auto& t : variants)
				root["variants"].append(utf_conv<char32_t, char>::convert(t));

			root["notations"].resize(0);
			for (const auto& t : notations)
				root["notations"].append(utf_conv<char32_t, char>::convert(t));

			root["translations"].resize(0);
			for (const auto& t : translations)
			{
				Json::Value trans;
				trans["id"] = std::get<0>(t);
				trans["lib_id"] = std::get<1>(t);
				trans["tag"] = utf_conv<char32_t, char>::convert(std::get<2>(t));
				trans["meaning"] = utf_conv<char32_t, char>::convert(std::get<3>(t));
				root["translations"].append(trans);
			}

			root["pronunciations"].resize(0);
			for (const auto& t : pronunciations)
				root["pronunciations"].append(t);

			root["sentences"].resize(0);
			for (const auto& t : sentences)
			{
				Json::Value trans;
				trans["id"] = std::get<0>(t);
				trans["trans_id"] = std::get<1>(t);
				root["sentences"].append(trans);
			}

			root["showing_time"] = showing_time;
			root["n_skips"] = n_skips;
			root["n_flick"] = n_flick;
			root["n_pause"] = n_pause;
			root["n_pronounce"] = n_pronounce;
			root["n_query"] = n_query;

			return Json::write(root);
		}
		virtual void from_string(std::u8string_view str) override
		{
			Json::Value root = Json::read(str);

			try
			{
				id = root["id"].asUInt64();
				origin = utf_conv<char, char32_t>::convert(root["origin"].asCString());

				variants.resize(root["variants"].size());
				for (size_t i = 0; i < variants.size(); i++)
					variants[i] = utf_conv<char, char32_t>::convert(
						root["variants"][static_cast<Json::ArrayIndex>(i)].asCString());

				notations.resize(root["notations"].size());
				for (size_t i = 0; i < notations.size(); i++)
					notations[i] = utf_conv<char, char32_t>::convert(
						root["notations"][static_cast<Json::ArrayIndex>(i)].asCString());

				translations.resize(root["translations"].size());
				for (size_t i = 0; i < translations.size(); i++)
				{
					const Json::Value& node = root["translations"][static_cast<Json::ArrayIndex>(i)];
					translations[i] = { node["id"].asUInt64(),
						node["lib_id"].asUInt64(),
						utf_conv<char, char32_t>::convert(node["tag"].asCString()),
						utf_conv<char, char32_t>::convert(node["meaning"].asCString()) };
				}

				pronunciations.resize(root["pronunciations"].size());
				for (size_t i = 0; i < pronunciations.size(); i++)
					pronunciations[i] = root["pronunciations"][static_cast<Json::ArrayIndex>(i)].asUInt64();

				sentences.resize(root["sentences"].size());
				for (size_t i = 0; i < sentences.size(); i++)
				{
					const Json::Value& node = root["sentences"][static_cast<Json::ArrayIndex>(i)];
					sentences[i] = { node["id"].asUInt64(),
						node["trans_id"].asUInt64() };
				}

				showing_time = root["showing_time"].asUInt64();
				n_skips = root["n_skips"].asUInt64();
				n_flick = root["n_flick"].asUInt64();
				n_pause = root["n_pause"].asUInt64();
				n_pronounce = root["n_pronounce"].asUInt64();
				n_query = root["n_query"].asUInt64();
			}
			catch (...)
			{
				throw deserialize_error("fail to translate the string into raw_item.");
			}
		}
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