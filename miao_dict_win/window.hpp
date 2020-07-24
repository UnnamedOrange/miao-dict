/// <summary>
/// 封装的窗口基类。包含此文件即可使用。
/// </summary>

#pragma once

#include <stdexcept>
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
};