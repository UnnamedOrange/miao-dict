#pragma once

#include "include.hpp"
#include "item.hpp"

namespace miao::core
{
	class library
	{
	public:
		id_t id;
		std::u32string tag;
		std::u32string lang;

		// dict
		std::vector<item> items;
		std::vector<raw_item> raw_items;

		// raw
		std::vector<std::u32string> passages;

		// sentences
		std::vector<std::tuple<id_t, std::u32string>> sentences;

		// pronunciations
		// TODO
	};
}