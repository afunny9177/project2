/*!
 * \file CallSection.cpp
 *
 * \author Administrator
 * \date 七月 2014
 *
 * 项目主文件，你可以在这里实现声明的导出函数等
 */

#include "stdafx.h"
#include "CallSection.h"
#include "pe_image.h"

typedef DWORD (WINAPI *install_thread_func)(LPVOID p);
typedef int (WINAPI *sumsecsfield_func)(int idx);
WNDPROC g_oldWndProc = nullptr;
int g_OnProgressCallback = 0;
ITaskbarList3 *g_pTaskbar = nullptr;
HWND g_hWnd = nullptr;

UINT_PTR PluginCallback(NSPIM msg)
{
    return 0;
}

bool EnumImportsCallback(const base::win::PEImage &image, LPCSTR module,
    DWORD ordinal, LPCSTR name, DWORD hint,
    PIMAGE_THUNK_DATA iat, PVOID cookie)
{
    if (lstrcmpiA(name, "CreateThread") == 0)
    {
        if (cookie != nullptr)
            *(LPVOID*)cookie= iat;

        return false;
    }

    return true;
}

extern "C" void CALL_SECTION_API call(HWND hWndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
    // 初始化插件和注册插件回调函数，一次即可
    NSIS_PLUGIN_INIT();
    extra->RegisterPluginCallback(g_hModule, PluginCallback);

    g_OnProgressCallback = popint();
    g_hWnd = hWndParent;

    LPVOID pCreateThread = nullptr;
    base::win::PEImage pe(GetModuleHandle(NULL));
    pe.EnumAllImports(EnumImportsCallback, &pCreateThread);

    if (pCreateThread == nullptr)
    {
        MessageBox(hWndParent, _T("查找地址失败，你用的NSIS是改过的？"), _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }

    // 找到.text节（即代码）所在的起始地址和结束地址
    auto t = pe.GetImageSectionHeaderByName(".text");
    BYTE *startAddr = (BYTE*) pe.RVAToAddr(t->VirtualAddress);
    BYTE *endAddr = (BYTE*) startAddr + t->Misc.VirtualSize;

    // install_thread函数的地址
    install_thread_func install_thread = nullptr;
    for (BYTE *addr = endAddr - 4; addr >= startAddr; addr--)
    {
        if (*(unsigned int*)addr == (unsigned int)pCreateThread)
        {
            // 68即push一个地址的指令
            BYTE *p = addr - 1;
            while (*p != 0x68)
                p--;
            
            install_thread = (install_thread_func) (*(int*)(p+1));
            endAddr = addr;
            break;
        }
    }

    if (install_thread == nullptr)
    {
        MessageBox(hWndParent, _T("查找失败！"), _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }

    // sumsecsfield函数的地址和progress_bar_len变量的地址
    sumsecsfield_func sumsecsfield = nullptr;
    LPVOID progress_bar_len = nullptr;
    for (BYTE *addr = endAddr; addr >= startAddr; addr--)
    {
        // 6A 04即push 0x4
        if (*(unsigned short*)addr == MAKEWORD(0x6A, 0x04))
        {
            if (*(addr + 2) == 0xE8)
            {
                sumsecsfield = (sumsecsfield_func)(int)((addr + 2 + 5) + *(unsigned int*)(addr + 3));
            }
            else if (*(unsigned short*)(addr + 2) == MAKEWORD(0xFF, 0x15))
            {
                sumsecsfield = (sumsecsfield_func)(*(unsigned int*)(addr + 4));
            }
            else
            {
                MessageBox(hWndParent, _T("查找失败！!"), _T("错误"), MB_OK | MB_ICONERROR);
                return;
            }

            BYTE *p = addr + 2 + 5;
            while (*p != 0xA3)
                p++;

            progress_bar_len = (LPVOID)(*(unsigned int*)(p + 1));
            break;
        }
    }

    if (sumsecsfield == nullptr || progress_bar_len == nullptr)
    {
        MessageBox(hWndParent, _T("查找失败！！！"), _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }

    // -----------------------------------------------------------------------------------------
    // 获取IID_ITaskbarList3的接口指针！win7或以上版本才有！！
    HRESULT hr = ::CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (LPVOID*)&g_pTaskbar);
    if (SUCCEEDED(hr) && g_pTaskbar != nullptr)
    {
        // 设置任务栏进度标识
        g_pTaskbar->SetProgressState(hWndParent, TBPF_NORMAL);
    }

    HWND hChildDialog = ::FindWindowEx(hWndParent, nullptr, _T("#32770"), nullptr);
    DWORD dwStyle = WS_CHILD | PBS_SMOOTH;
    HWND hwndProgressBar = CreateWindow(_T("msctls_progress32"), _T(""), dwStyle,
        0, 0, 0, 0, hChildDialog ? hChildDialog : hWndParent, (HMENU)1024, g_hModule, nullptr);

    // 子类化进度条
    g_oldWndProc = SubclassWindow(hwndProgressBar, ProgressbarWindowProc);
    
    // 设置进度最大值
    *(unsigned int*)progress_bar_len = sumsecsfield(4);

    // 设置进度条最大值
    SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 30000));

    DWORD id;
    CloseHandle(CreateThread(NULL, 0, install_thread, hwndProgressBar, 0, &id));
}

LRESULT CALLBACK ProgressbarWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // NSIS中进度条的值的范围是0-30000！
    if (uMsg == PBM_SETPOS)
    {
        int percent = (int)(wParam * 1.0 / 30000 * 100);

        if (g_pTaskbar != nullptr)
        {
            g_pTaskbar->SetProgressValue(g_hWnd, percent, 100);
        }

        if (g_OnProgressCallback != 0)
        {
            pushint(percent);
            g_extra->ExecuteCodeSegment(g_OnProgressCallback - 1, nullptr);
        }

        if (percent >= 100)
        {
            if (g_pTaskbar != nullptr)
            {
                g_pTaskbar->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
                g_pTaskbar->Release();
                g_pTaskbar = nullptr;
            }

            SubclassWindow(hwnd, g_oldWndProc);
        }
    }
    
    if (g_oldWndProc)
        return CallWindowProc(g_oldWndProc, hwnd, uMsg, wParam, lParam);
    else
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
}