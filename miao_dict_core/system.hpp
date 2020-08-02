#pragma once

#include "include.hpp"
#include "config.hpp"
#include "item.hpp"

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
			_working_dir = config::atom()->working_dir();
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

		/// <returns>工作目录/miao_dict。</returns>
		std::filesystem::path root_dir() const
		{
			return working_dir / "miao_dict";
		}
		/// <param name="id">库 id。</param>
		/// <returns>工作目录/miao_dict/library(/id)。</returns>
		std::filesystem::path library_dir(id_t id = std::numeric_limits<id_t>::max()) const
		{
			if (id == std::numeric_limits<id_t>::max())
				return root_dir() / "library";
			return root_dir() / "library" / std::to_string(id);
		}
		/// <returns>工作目录/miao_dict/sentence。</returns>
		std::filesystem::path sentence_dir() const
		{
			return root_dir() / "sentence";
		}

	private:
		/// <summary>
		/// 库 id 到库引用的映射。如果加载成功，这个映射一定非空，因为总存在一个本地库。该变量作为系统是否已初始化的判断依据。
		/// </summary>
		std::unordered_map<id_t, std::shared_ptr<library>> libraries;

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
	public:
		/// <summary>
		/// 在文件系统中初始化 miao_dict 系统。仅在第一次使用 miao_dict 时调用。如果重复调用且 force 指定为 true，只保证已经存在的系统不受影响，不保证文件夹下其他文件不受影响。
		/// </summary>
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

			// 创建本地库文件夹。
			if (!demand_directory(library_dir(0)))
				return false;

			// TODO: 创建配置文件。
			return true;
		}
	};
}