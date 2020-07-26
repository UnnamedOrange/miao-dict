/// <summary>
/// 封装的窗口基类。包含此文件即可使用。
/// </summary>

#pragma once

#include <string>
#include <unordered_set>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <typeinfo>
#include <Windows.h>

/// <summary>
/// window 基类。
/// </summary>
class window
{
private:
	HWND __hwnd{};
public:
	const HWND& hwnd{ __hwnd }; // 窗口句柄的常值引用。

private:
	RECT get_window_rect() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		RECT ret;
		if (!GetWindowRect(hwnd, &ret))
			throw std::runtime_error("fail to GetWindowRect.");
		return ret;
	}
public:
	/// <returns>
	/// 窗口左部坐标。
	/// </returns>
	int left() const
	{
		return get_window_rect().left;
	}
	/// <returns>
	/// 窗口顶部坐标。
	/// </returns>
	int top() const
	{
		return get_window_rect().top;
	}
	/// <returns>
	/// 窗口右部坐标。
	/// </returns>
	int right() const
	{
		return get_window_rect().right;
	}
	/// <returns>
	/// 窗口底部坐标。
	/// </returns>
	int bottom() const
	{
		return get_window_rect().bottom;
	}
	/// <returns>
	/// 窗口宽度。
	/// </returns>
	int width() const
	{
		auto t = get_window_rect();
		return t.right - t.left;
	}
	/// <returns>
	/// 窗口高度。
	/// </returns>
	int height() const
	{
		auto t = get_window_rect();
		return t.bottom - t.top;
	}
private:
	void set_window_rect(const RECT& r)
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		if (!MoveWindow(hwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, true))
			throw std::runtime_error("fail to MoveWindow.");
	}
public:
	/// <summary>
	/// 设置窗口左部坐标。
	/// </summary>
	int left(int x)
	{
		auto r = get_window_rect();
		r.left = x;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口顶部坐标。
	/// </summary>
	int top(int y)
	{
		auto r = get_window_rect();
		r.top = y;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口右部坐标。
	/// </summary>
	int right(int x)
	{
		auto r = get_window_rect();
		r.right = x;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口底部坐标。
	/// </summary>
	int bottom(int y)
	{
		auto r = get_window_rect();
		r.bottom = y;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口宽度，保持窗口左部不移动。
	/// </summary>
	int width(int cx)
	{
		auto r = get_window_rect();
		r.right = r.left + cx;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口高度，保持窗口顶部不移动。
	/// </summary>
	int height(int cy)
	{
		auto r = get_window_rect();
		r.bottom = r.top + cy;
		set_window_rect(r);
	}
private:
	/// <returns>
	/// 窗口客户区在屏幕上对应的矩形。
	/// </returns>
	RECT get_client_rect() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		RECT ret;
		if (!GetClientRect(hwnd, &ret))
			throw std::runtime_error("fail to GetWindowRect.");
		POINT pt;
		if (!ClientToScreen(hwnd, &pt))
			throw std::runtime_error("fail to ClientToScreen.");
		ret.left += pt.x;
		ret.right += pt.x;
		ret.top += pt.y;
		ret.bottom += pt.y;
		return ret;
	}
public:
	/// <returns>
	/// 窗口客户区左部坐标。
	/// </returns>
	int cleft() const
	{
		return get_client_rect().left;
	}
	/// <returns>
	/// 窗口客户区顶部坐标。
	/// </returns>
	int ctop() const
	{
		return get_client_rect().top;
	}
	/// <returns>
	/// 窗口客户区右部坐标。
	/// </returns>
	int cright() const
	{
		return get_client_rect().right;
	}
	/// <returns>
	/// 窗口客户区底部坐标。
	/// </returns>
	int cbottom() const
	{
		return get_client_rect().bottom;
	}
	/// <returns>
	/// 窗口客户区宽度。
	/// </returns>
	int cwidth() const
	{
		auto t = get_client_rect();
		return t.right - t.left;
	}
	/// <returns>
	/// 窗口客户区高度。
	/// </returns>
	int cheight() const
	{
		auto t = get_client_rect();
		return t.bottom - t.top;
	}

public:
	/// <returns>
	/// 注册窗口时（如果需要）的窗口类名。可以用作窗口的标识。
	/// </returns>
	std::string get_class_name() const
	{
		std::string_view name = typeid(*this).name();
		std::string ret;
		bool st = false;
		for (size_t i = name.length() - 1; ~i; i--)
			if (name[i] == ' ')
			{
				if (st)	break;
			}
			else
			{
				st = true;
				ret.push_back(name[i]);
			}
		std::reverse(ret.begin(), ret.end());
		return ret;
	}

private:
	inline static std::mutex mutex_create; // 创建窗口时所用的互斥体，创建窗口的过程必须是串行的。
	inline static std::condition_variable cv_create; // // 创建窗口时所用的条件变量，用于模拟信号量。创建窗口的过程必须是串行的。
	inline static window* storage{}; // 创建窗口时所用的全局变量。

private:
	void register_class()
	{
		static std::unordered_set<std::string> registered;
		auto name = get_class_name();
		if (registered.count(name))
			return;

		WNDCLASSEXA wcex{ sizeof(WNDCLASSEXW) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = VirtualWindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandleW(nullptr);
		wcex.hIcon = nullptr;
		wcex.hIconSm = nullptr;
		wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = name.c_str();
		if (!RegisterClassExA(&wcex))
			throw std::runtime_error("fail to RegisterClassExA.");
		registered.insert(name);
	}
	static LRESULT CALLBACK VirtualWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		window* p = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		if (!p)
		{
			if (!storage)
				throw std::runtime_error("storage shouldn't be nullptr.");
			p = storage;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)p);
			p->__hwnd = hwnd;

			storage = nullptr;
			cv_create.notify_one();
		}
		LRESULT ret = p->WindowProc(hwnd, message, wParam, lParam);

		if (p && message == WM_DESTROY)
			p->__hwnd = nullptr;

		return ret;
	}

protected:
	virtual INT_PTR WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

public:
	/// <summary>
	/// 使用 CreateWindowExA 创建窗口。
	/// </summary>
	HWND create(HWND hwndParent = nullptr)
	{
		std::unique_lock<std::mutex> lock(mutex_create);
		cv_create.wait(lock, []()->bool
			{
				return !storage;
			});
		storage = this;
		lock.unlock();

		register_class();
		return CreateWindowExA(0,
			get_class_name().c_str(),
			"",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwndParent,
			nullptr,
			GetModuleHandleW(nullptr),
			nullptr);
	}

public:
	/// <summary>
	/// 开始窗口消息循环。
	/// </summary>
	/// <returns>
	/// 退出消息的返回值。
	/// </returns>
	int message_loop()
	{
		MSG msg;
		while (GetMessageW(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return (int)msg.wParam;
	}
};