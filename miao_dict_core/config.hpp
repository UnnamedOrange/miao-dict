#pragma once

#include "include.hpp"

namespace miao::core
{
	template <typename T, typename mutex_t = std::mutex>
	class lock_view
	{
		T& ref;
		mutex_t& mutex;

	public:
		lock_view() = delete;
		lock_view(const lock_view&) = delete;
		lock_view(lock_view&&) = default;
		lock_view& operator=(const lock_view&) = delete;
		lock_view& operator=(lock_view&&) = default;
		lock_view(T& ref, mutex_t& mutex) : ref(ref), mutex(mutex)
		{
			mutex.lock();
		}
		~lock_view()
		{
			mutex.unlock();
		}

		/// <summary>
		/// 访问对应对象的属性。
		/// </summary>
		T* operator->()
		{
			return &ref;
		}
		/// <summary>
		/// 返回对应对象的引用。
		/// </summary>
		T& operator*()
		{
			return ref;
		}
	};
	template <typename T, typename mutex_t = std::mutex>
	class lock_view_maker
	{
		mutex_t mutex;
	public:
		lock_view<T, mutex_t> make_lock_view(T& ref)
		{
			return lock_view<T, mutex_t>(ref, mutex);
		}
	};
	class config final : protected Json::Value, public serializable_base
	{
	public:
		[[nodiscard]] virtual Json::Value to_json() const
		{
			return *this;
		}
		virtual void from_json(const Json::Value& value)
		{
			Json::Value::operator=(value);
		}

	public:
		/// <summary>
		/// 返回一个自动为配置对象加锁的视图。如果接下来要在对象上进行一系列操作，应当将返回值保存在一个变量中。同一线程中不可同时存在两个这样的视图，否则会产生死锁。
		/// </summary>
		/// <returns>自动加锁视图。</returns>
		[[nodiscard]] static lock_view<config> view()
		{
			static lock_view_maker<config> lv_maker;
			static config _;
			return lv_maker.make_lock_view(_);
		}
	private:
		config()
		{
			try
			{
				from_file("miao_dict_config.json");
			}
			catch (const std::runtime_error&) // 文件不存在。
			{
				Json::Value::operator[]("working_dir") = reinterpret_cast<const char*>(U""_u8.c_str());
			}
		}
	public:
		~config()
		{
			to_file("miao_dict_config.json");
		}
		config(const config&) = delete;
		config(config&&) = delete;
		config& operator=(const config&) = delete;
		config& operator=(config&&) = delete;

	public:
		[[nodiscard]] std::filesystem::path working_dir() const
		{
			return std::filesystem::path(utf_conv<char, char32_t>::convert(Json::Value::operator[]("working_dir").asCString()));
		}
		void working_dir(std::filesystem::path new_dir)
		{
			Json::Value::operator[]("working_dir") = utf_conv<char32_t, char>::convert(new_dir.u32string());
		}
	};
}