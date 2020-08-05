#pragma once

#include "include.hpp"
#include "config.hpp"
#include "library.hpp"

namespace miao::core
{
	class system final
	{
	private:
		inline static system* __address_instance{};
	public:
		/// <summary>
		/// 是否存在 system 类的实例。
		/// </summary>
		/// <returns> 若存在，即 __address_instance 非零，则返回 true，否则返回 false。 </returns>
		static bool exists_instance()
		{
			return __address_instance;
		}
		/// <summary>
		/// 获取 system 类的唯一实例。
		/// </summary>
		/// <returns> 若存在 system 类的实例，则返回其引用，否则抛出 std::runtime_error 异常。 </returns>
		static system& instance()
		{
			if (!__address_instance)
				throw std::runtime_error("__address_instance should not be nullptr.");
			return *__address_instance;
		}

	public:
		/// <summary>
		/// 默认构造函数。只能存在一个实例，否则会抛出 std::runtime_error 异常。
		/// </summary>
		/// <returns></returns>
		system()
		{
			if (__address_instance)
				throw std::runtime_error("__address_instance should be nullptr.");
			__address_instance = this;

			construct();
		}
		~system()
		{
			__address_instance = nullptr;
		}
		system(const system&) = delete;
		system(system&&) = delete;
		system& operator=(const system&) = delete;
		system& operator=(system&&) = delete;

	private:
		/// <summary>
		/// 在构造函数中调用，用于初始化较复杂对象或全局设置相关对象。
		/// </summary>
		void construct()
		{
			auto cfg = config::view();
			set_working_dir(cfg->working_dir());
		}

	private:
		std::filesystem::path _working_dir; // 工作目录。
	public:
		const std::filesystem::path& working_dir{ _working_dir }; // 工作目录。
		/// <summary>
		/// 设置工作目录。会自动设置路径中的分隔符。
		/// </summary>
		/// <param name="path">新的工作目录。</param>
		void set_working_dir(std::filesystem::path path)
		{
			path.make_preferred();
			_working_dir = path;
		}

		/// <returns>
		/// 工作目录/miao_dict。
		/// </returns>
		std::filesystem::path root_dir() const
		{
			return working_dir / "miao_dict";
		}
		/// <param name="id">库 id。</param>
		/// <returns>
		/// 工作目录/miao_dict/library(/id)。
		/// </returns>
		std::filesystem::path library_dir(id_t id = std::numeric_limits<id_t>::max()) const
		{
			if (id == std::numeric_limits<id_t>::max())
				return root_dir() / "library";
			return root_dir() / "library" / std::to_string(id);
		}
		/// <returns>
		/// 工作目录/miao_dict/sentence。
		/// </returns>
		std::filesystem::path sentence_dir() const
		{
			return root_dir() / "sentence";
		}

	private:
		/// <summary>
		/// 要求指定路径是一个存在的目录。
		/// </summary>
		/// <param name="path">指定的路径。</param>
		/// <returns>如果调用该函数后满足条件，则返回 true，否则返回 false。</returns>
		static bool demand_directory(std::filesystem::path path)
		{
			if (std::filesystem::exists(path) && !std::filesystem::is_directory(path)) // 不是一个目录，失败。
				return false;
			try
			{
				if (!std::filesystem::exists(path) && !std::filesystem::create_directory(path)) // 无法创建目录，失败。
					return false;
			}
			catch (const std::filesystem::filesystem_error&)
			{
				return false;
			}
			return true;
		}
		/// <summary>
		/// 要求指定路径是一个存在的文件。如果本身存在，则新建一个空文件。
		/// </summary>
		/// <param name="path">指定的路径。</param>
		/// <returns>如果调用该函数后满足条件，则返回 true，否则返回 false。</returns>
		static bool demand_file(std::filesystem::path path)
		{
			if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) // 不是一个目录，失败。
				return false;
			try
			{
				if (!std::filesystem::exists(path))
				{
					std::ofstream _(path);
					if (!std::filesystem::exists(path)) // 无法创建目录，失败。
						return false;
				}
			}
			catch (const std::filesystem::filesystem_error&)
			{
				return false;
			}
			return true;
		}
		/// <summary>
		/// 列出目录下所有的 json 文件，按字典序排序。不会递归搜索。
		/// </summary>
		/// <param name="path">目录。</param>
		static std::vector<std::filesystem::path> list_json_files(std::filesystem::path path)
		{
			std::vector<std::filesystem::path> ret;
			for (auto& p : std::filesystem::directory_iterator(path))
				if (!p.is_directory() && p.path().extension() == ".json")
					ret.push_back(p.path());
			std::sort(ret.begin(), ret.end());
			return ret;
		}
		/// <summary>
		/// 列出目录下所有的文件夹，按字典序排序。不会递归搜索。
		/// </summary>
		/// <param name="path">目录。</param>
		static std::vector<std::filesystem::path> list_directories(std::filesystem::path path)
		{
			std::vector<std::filesystem::path> ret;
			for (auto& p : std::filesystem::directory_iterator(path))
				if (p.is_directory())
					ret.push_back(p.path());
			std::sort(ret.begin(), ret.end());
			return ret;
		}
	public:
		/// <summary>
		/// 在文件系统中初始化 miao_dict 系统。仅在第一次使用 miao_dict 时调用。如果重复调用且 force 指定为 true，只保证已经存在的系统不受影响，不保证文件夹下其他文件不受影响。
		/// </summary>
		/// <remarks>现在在调用 load 前会自动调用 init(true)。如果返回 false，则 load 直接失败。</remarks>
		/// <param name="force">如果参数 force 为真，则在已经存在文件夹 "working_dir/miao_dict" 时仍然初始化整个系统；否则在这种情况下直接返回 false。</param>
		/// <returns>如果成功初始化，则返回 true，否则返回 false。如果返回 false，则不保证已经创建文件的完整性。</returns>
		bool init(bool force = false)
		{
			// 创建根文件夹。
			if (!force && std::filesystem::exists(root_dir()))
				return false;
			if (!demand_directory(root_dir()))
				return false;

			// 创建库文件夹。
			if (!demand_directory(library_dir()))
				return false;

			// 创建句子文件夹。
			if (!demand_directory(sentence_dir()))
				return false;

			return true;
		}

	private:
		/// <summary>
		/// 库 id 到库引用的映射。如果加载成功，这个映射一定非空，因为总存在一个本地库。该变量作为系统是否已初始化的判断依据。
		/// </summary>
		std::map<id_t, std::shared_ptr<library>> libraries;
	public:
		/// <summary>
		/// 加载所有库和附属信息到内存中。如果这个对象已经加载的信息非空，则无论函数是否成功，都将被全部抛弃。
		/// </summary>
		/// <returns>如果加载成功，返回 true；否则返回 false。</returns>
		bool load()
		{
			// 创建基本的文件夹。
			if (!init(true))
				return false;

			// 抛弃全部已经加载到内存中的库及附属信息。
			libraries.clear();

			// 加载本地库。
			if (!demand_library(0))
				return false;
			libraries[0] = std::make_shared<library>(load_library(0));
			libraries[0]->lang = config::view()->preferred_lang();
			libraries[0]->tag = U"local";
			libraries[0]->to_file(library_dir(0) / "library.json");

			// 加载其他库。
			auto lib_dirs = list_directories(library_dir());
			for (const auto& p : lib_dirs)
			{
				id_t id{};
				try
				{
					id = std::stoi(p.filename());
				}
				catch (const std::invalid_argument&)
				{
					continue;
				}

				if (!id)
					continue;
				if (!demand_library(id))
					continue;
				libraries[id] = std::make_shared<library>(load_library(id));
			}

			return true;
		}

	private:
		/// <summary>
		/// 要求指定路径是一个合法的存有 item 的文件。该函数会尝试修复文件中缺失的信息（如新版本中的信息），并在尝试修复后会重写这个文件。
		/// </summary>
		/// <param name="p">指定路径。</param>
		/// <returns>如果返回 true，则保证此时文件内容能够完全被正确加载。否则返回 false。</returns>
		bool demand_item(std::filesystem::path p)
		{
			item ti;
			try
			{
				ti.from_file(p);
			}
			catch (const parse_error&) // 认为该文件损坏，直接失败。
			{
				return false;
			}
			catch (const deserialize_error&) // 在之后检查 ver_tag。
			{

			}
			catch (const std::runtime_error&) // 未知的其他错误，直接失败。
			{
				return false;
			}

			bool need_repair = ti.ver_tag != ti.latest_ver_tag;
			try
			{
				id_t nid = std::stoi(p.filename().replace_extension());
				need_repair |= ti.id != nid;
				ti.id = nid; // 修复 id。
			}
			catch (const std::invalid_argument&) // 文件名不表示一个有效 id。
			{
				return false;
			}

			if (ti.ver_tag < 1)
			{
				if (ti.origin.empty()) // 空单词，直接失败。
					return false;
				ti.ver_tag = 1;
			}

			if (need_repair)
				ti.to_file(p); // 重写入。
			return true;
		}
		/// <summary>
		/// 要求指定路径是一个合法的存有 passage 的文件。该函数会尝试修复文件中缺失的信息（如新版本中的信息），并在尝试修复后会重写这个文件。
		/// </summary>
		/// <param name="p">指定路径。</param>
		/// <returns>如果返回 true，则保证此时文件内容能够完全被正确加载。否则返回 false。</returns>
		bool demand_passage(std::filesystem::path p)
		{
			passage tp;
			try
			{
				tp.from_file(p);
			}
			catch (const parse_error&) // 认为该文件损坏，直接失败。
			{
				return false;
			}
			catch (const deserialize_error&) // 在之后检查 ver_tag。
			{

			}
			catch (const std::runtime_error&) // 未知的其他错误，直接失败。
			{
				return false;
			}

			bool need_repair = tp.ver_tag != tp.latest_ver_tag;
			try
			{
				id_t nid = std::stoi(p.filename().replace_extension());
				need_repair |= tp.id != nid;
				tp.id = nid; // 修复 id。
			}
			catch (const std::invalid_argument&) // 文件名不表示一个有效 id。
			{
				return false;
			}

			if (tp.ver_tag < 1)
			{
				if (tp.content.empty()) // 空文章，直接失败。
					return false;
				tp.ver_tag = 1;
			}
			if (tp.ver_tag < 2)
			{
				tp.ver_tag = 2;
			}

			if (need_repair)
				tp.to_file(p); // 重写入。
			return true;
		}
		/// <summary>
		/// 要求指定路径是一个合法的存有 raw_item 的文件。该函数会尝试修复文件中缺失的信息，并在尝试修复后会重写这个文件。
		/// </summary>
		/// <param name="p">指定路径。</param>
		/// <returns>如果返回 true，则保证此时文件内容能够完全被正确加载。否则返回 false。</returns>
		bool demand_raw_items(std::filesystem::path p)
		{
			Json::Value v;
			bool need_repair = false;
			try
			{
				std::ifstream ifs(p);
				ifs.seekg(0, std::ios::end);
				size_t len = ifs.tellg();
				ifs.seekg(0, std::ios::beg);
				std::vector<char8_t> buf(len + 1);
				ifs.read(reinterpret_cast<char*>(buf.data()), len);
				v = Json::read(buf.data());
			}
			catch (...) // 无法解析。
			{
				v["raw_items"].resize(0);
				need_repair = true;
			}

			if (v["raw_items"].type() != Json::arrayValue)
			{
				v["raw_items"].clear();
				v["raw_items"].resize(0);
				need_repair = true;
			}

			for (Json::ArrayIndex i = 0; i < v["raw_items"].size(); i++)
			{
				raw_item ri;
				auto& rij = v["raw_items"][i];
				try
				{
					ri.from_json(rij);
				}
				catch (const deserialize_error&) // 在之后检查 ver_tag。
				{

				}
				catch (const std::runtime_error&) // 未知的其他错误，直接失败。
				{
					continue;
				}

				need_repair |= ri.ver_tag != ri.latest_ver_tag;

				if (ri.ver_tag < 1)
				{
					if (ri.origin.empty())
						continue;
					ri.ver_tag = 1;
				}
			}

			if (need_repair) // 重写入。
			{
				std::ofstream fs(p);
				std::u8string str = Json::write(v);
				fs.write(reinterpret_cast<char*>(str.data()), str.length());
			}
			return true;
		}
		/// <summary>
		/// 要求指定路径是一个合法的存有库配置的文件。该函数会尝试修复文件中缺失的信息，并在尝试修复后会重写这个文件。
		/// </summary>
		/// <param name="p">指定路径。</param>
		/// <returns>如果返回 true，则保证此时文件内容能够完全被正确加载。否则返回 false。</returns>
		bool demand_library_config(std::filesystem::path p)
		{
			library tl;
			try
			{
				tl.from_file(p);
			}
			catch (const parse_error&) // 认为该文件损坏，重新创建。
			{
				tl.ver_tag = 0;
			}
			catch (const deserialize_error&) // 在之后检查 ver_tag。
			{

			}
			catch (const std::runtime_error&) // 未知的其他错误，直接失败。
			{
				return false;
			}

			bool need_repair = tl.ver_tag != tl.latest_ver_tag;
			try
			{
				id_t nid = std::stoi(p.parent_path().filename());
				need_repair |= tl.id != nid;
				tl.id = nid; // 修复 id。
			}
			catch (const std::invalid_argument&) // 路径名不表示一个有效 id。
			{
				return false;
			}

			if (tl.ver_tag < 1)
			{
				if (tl.lang.empty())
					tl.lang = U"custom";
				tl.ver_tag = 1;
			}

			if (need_repair)
				tl.to_file(p); // 重写入。
			return true;
		}
		/// <summary>
		/// 要求指定路径是一个合法的库路径。该函数会尝试修复库中缺失的信息（如缺失的目录、文件）。
		/// </summary>
		/// <param name="id">指定路径</param>
		/// <returns>如果返回 true，则保证此时库能够完全被正确加载。否则返回 false。</returns>
		bool demand_library(id_t id)
		{
			auto lib_dir = library_dir(id);
			if (!demand_directory(lib_dir))
				return false;
			if (!demand_directory(lib_dir / "items"))
				return false;
			if (!demand_directory(lib_dir / "pronunciations"))
				return false;
			if (!demand_directory(lib_dir / "passages"))
				return false;

			if (!demand_file(lib_dir / "raw_items.json"))
				return false;
			if (!demand_file(lib_dir / "library.json"))
				return false;

			auto items_path = list_json_files(lib_dir / "items");
			for (const auto& p : items_path)
				demand_item(p);

			auto passages_path = list_json_files(lib_dir / "passages");
			for (const auto& p : passages_path)
				demand_passage(p);

			if (!demand_raw_items(lib_dir / "raw_items.json"))
				return false;

			demand_library_config(lib_dir / "library.json");

			return true;
		}

		/// <summary>
		/// 给定存有 raw_item 的文件，加载所有的 raw_item。如果产生解析错误，会直接抛出异常；对于所有不正常的 raw_item，会直接跳过。需要先调用 demand_raw_items。
		/// </summary>
		/// <param name="p">指定文件的路径。</param>
		/// <returns>存有 raw_item 的数组。</returns>
		std::vector<raw_item> load_raw_items(std::filesystem::path p)
		{
			std::vector<raw_item> ret;
			Json::Value v;
			{
				std::ifstream ifs(p);
				ifs.seekg(0, std::ios::end);
				size_t len = ifs.tellg();
				ifs.seekg(0, std::ios::beg);
				std::vector<char8_t> buf(len + 1);
				ifs.read(reinterpret_cast<char*>(buf.data()), len);
				v = Json::read(buf.data());
			}

			for (Json::ArrayIndex i = 0; i < v["raw_items"].size(); i++)
			{
				raw_item ri;
				auto& rij = v["raw_items"][i];
				try
				{
					ri.from_json(rij);
				}
				catch (...)
				{
					continue;
				}
				if (ri.ver_tag != ri.latest_ver_tag)
					continue;
				ret.push_back(ri);
			}
			return ret;
		}
	public:
		/// <summary>
		/// 加载指定的库。应当先调用 demand_library。
		/// </summary>
		/// <param name="id">库 id。</param>
		/// <returns>被加载的库对象。</returns>
		library load_library(id_t id)
		{
			auto lib_dir = library_dir(id);
			library ret;

			ret.from_file(lib_dir / "library.json");

			ret.items.clear();
			auto items_path = list_json_files(lib_dir / "items");
			for (const auto& p : items_path)
			{
				try
				{
					item ti;
					ti.from_file(p);
					if (ti.ver_tag != ti.latest_ver_tag)
						continue;
					ret.items[ti.id] = ti;
				}
				catch (...)
				{
					continue;
				}
			}

			auto passages_path = list_json_files(lib_dir / "passages");
			for (const auto& p : passages_path)
			{
				try
				{
					passage tp;
					tp.from_file(p);
					if (tp.ver_tag != tp.latest_ver_tag)
						continue;
					ret.passages.push_back(tp);
				}
				catch (...)
				{
					continue;
				}
			}

			ret.raw_items = load_raw_items(lib_dir / "raw_items.json");

			return ret;
		}

	public:
		/// <summary>
		/// 返回一个未使用的库 id。总是应当在加载库后调用，否则抛出 std::runtime_error 异常。
		/// </summary>
		/// <returns></returns>
		id_t get_free_library_id() const
		{
			if (libraries.empty())
				throw std::runtime_error("call load() before get_free_library_id.");
			return libraries.crbegin()->first + 1;
		}
		/// <summary>
		/// 创建指定 id 的空库，如果库已经存在则失败。会生成所需要的文件。总是应当在加载库后调用，否则抛出 std::runtime_error 异常。
		/// </summary>
		/// <param name="id">库 id。如果是 std::nullopt，则使用 get_free_library_id 的结果。</param>
		/// <param name="lang">语言标签。</param>
		/// <param name="tag">库名。</param>
		/// <returns>成功返回 true，失败返回 false。成功后，可以通过 libraries 进行访问。</returns>
		bool create_library(std::optional<id_t> id, std::u32string_view lang, std::u32string_view tag)
		{
			if (libraries.empty())
				throw std::runtime_error("call load() before create_library.");

			library tl;
			if (id)
				tl.id = *id;
			else
				tl.id = get_free_library_id();

			if (libraries.count(tl.id))
				return false;

			tl.lang = lang;
			tl.tag = tag;

			if (!demand_library(tl.id))
				return false;

			tl.to_file(library_dir(tl.id) / "library.json");

			libraries[tl.id] = std::make_shared<library>(std::move(tl));
			return true;
		}
	};
}