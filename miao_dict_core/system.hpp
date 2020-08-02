#pragma once

#include "include.hpp"
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
	};
}