/*!
 * \file CallSection.h
 *
 * \author Administrator
 * \date 七月 2014
 *
 * 项目主头文件，你可以在这里声明导出函数等
 */

#ifndef __CALL_SECTION_H__
#define __CALL_SECTION_H__

#ifdef CALL_SECTION_EXPORTS
#define CALL_SECTION_API __declspec(dllexport)
#else
#define CALL_SECTION_API __declspec(dllimport)
#endif

#include "pluginapi.h"

// 注册插件回调用，以便能自己加载/卸载
extern HMODULE g_hModule;

// 插件回调
UINT_PTR PluginCallback(NSPIM msg);

LRESULT CALLBACK ProgressbarWindowProc(HWND hwnd, UINT uMsg,WPARAM wParam, LPARAM lParam);

// 导出的必须是C函数，且函数签名是这样的：void f(HWND, int, THCAR*, stack_t**, extra_parameters*)
// 此为示例导出函数，在NSIS中这样调用（有一个参数）：CallSection::fun "hello nsis"
extern "C" void CALL_SECTION_API _(HWND hWndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra);

#endif
