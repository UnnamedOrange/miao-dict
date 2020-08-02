#pragma once

#include "include.hpp"

namespace miao::core
{
	template <typename T>
	class lock_view
	{
		T& ref;
		std::mutex& mutex;

	public:
		lock_view() = delete;
		lock_view(const lock_view&) = delete;
		lock_view(lock_view&&) = default;
		lock_view& operator=(const lock_view&) = delete;
		lock_view& operator=(lock_view&&) = default;
		lock_view(T& ref, std::mutex& mutex) : ref(ref), mutex(mutex)
		{
			mutex.lock();
		}
		~lock_view()
		{
			mutex.unlock();
		}

		T* operator->()
		{
			return &ref;
		}
		T& view()
		{
			return ref;
		}
	};
	template <typename T>
	class lock_view_maker
	{
		std::mutex mutex;
	public:
		lock_view<T> make_lock_view(T& ref)
		{
			return lock_view<T>(ref, mutex);
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
		static lock_view<config> atom()
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