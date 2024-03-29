﻿#pragma once

#include "include.hpp"

namespace miao::core
{
	class item : public serializable_base
	{
	public:
		// 版本标记
		static constexpr int latest_ver_tag = 1;
		int ver_tag = latest_ver_tag;

		// 基础数据
		id_t id{};
		std::u32string origin;
		std::vector<std::u32string> variants;
		std::vector<std::u32string> notations;
		std::vector<std::tuple<id_t, id_t, std::u32string, std::u32string>> translations; // (id, lib_id, tag, meaning)
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
		[[nodiscard]] virtual Json::Value to_json() const override
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

			return root;
		}
		virtual void from_json(const Json::Value& value) override
		{
			ver_tag = 0;
			try
			{
				Json::value_assign(origin, value["origin"]);
				id = value["id"].asUInt64();

				variants.resize(value["variants"].size());
				for (size_t i = 0; i < variants.size(); i++)
					Json::value_assign(variants[i], value["variants"][static_cast<Json::ArrayIndex>(i)]);

				notations.resize(value["notations"].size());
				for (size_t i = 0; i < notations.size(); i++)
					Json::value_assign(notations[i], value["notations"][static_cast<Json::ArrayIndex>(i)]);

				translations.resize(value["translations"].size());
				for (size_t i = 0; i < translations.size(); i++)
				{
					const Json::Value& node = value["translations"][static_cast<Json::ArrayIndex>(i)];
					translations[i] = { node["id"].asUInt64(),
						node["lib_id"].asUInt64(),
						std::get<std::u32string>(Json::value_cast(node["tag"])),
						std::get<std::u32string>(Json::value_cast(node["meaning"])) };
				}

				sentences.resize(value["sentences"].size());
				for (size_t i = 0; i < sentences.size(); i++)
				{
					const Json::Value& node = value["sentences"][static_cast<Json::ArrayIndex>(i)];
					sentences[i] = { node["id"].asUInt64(),
						node["trans_id"].asUInt64() };
				}

				showing_time = value["showing_time"].asUInt64();
				n_skips = value["n_skips"].asUInt64();
				n_flick = value["n_flick"].asUInt64();
				n_pause = value["n_pause"].asUInt64();
				n_pronounce = value["n_pronounce"].asUInt64();
				n_query = value["n_query"].asUInt64();

				ver_tag = 1;
			}
			catch (...)
			{
				if (!ver_tag)
					throw deserialize_error("fail to translate the json into raw_item.");
			}
		}
	};

	class raw_item final : public serializable_base
	{
	public:
		// 版本标记
		static constexpr int latest_ver_tag = 1;
		int ver_tag = latest_ver_tag;

		// 数据域
		std::u32string origin;
		uint_t frequency{};

	public:
		[[nodiscard]] virtual Json::Value to_json() const override
		{
			Json::Value root;
			root["frequency"] = frequency;
			root["origin"] = utf_conv<char32_t, char>::convert(origin);
			return root;
		}
		virtual void from_json(const Json::Value& value) override
		{
			ver_tag = 0;
			try
			{
				Json::value_assign(origin, value["origin"]);
				frequency = value["frequency"].asUInt64();
				ver_tag = 1;
			}
			catch (...)
			{
				if (!ver_tag)
					throw deserialize_error("fail to translate the json into raw_item.");
			}
		}
	};
}