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
		std::unordered_map<id_t, std::shared_ptr<library>> libraries;
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
			demand_library(0);

			return true;
		}
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
				ti.id = std::stoi(p.filename().replace_extension()); // 修复 id。
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
				tp.id = std::stoi(p.filename().replace_extension()); // 修复 id。
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

			if (!demand_file(lib_dir / "raws.json"))
				return false;
			if (!demand_file(lib_dir / "raws_items.json"))
				return false;
			if (!demand_file(lib_dir / "library.json"))
				return false;

			auto items_path = list_json_files(lib_dir / "items");
			for (const auto& p : items_path)
				demand_item(p);

			auto passages_path = list_json_files(lib_dir / "passages");
			for (const auto& p : passages_path)
				demand_passage(p);
			return true;
		}
	};
}