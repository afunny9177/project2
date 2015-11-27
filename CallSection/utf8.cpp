#include "StdAfx.h"
#include "utf8.h"

///////////////////////////////////////////////////////////////////////////////////
// 通用函数

wchar_t* C2W936(const char* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    int len = ::MultiByteToWideChar(936, 0, str, -1, NULL, 0);

    wchar_t *wstr = new wchar_t[len];
    ::ZeroMemory(wstr, len * 2);

    ::MultiByteToWideChar(936, 0, str, -1, wstr, len);

    return wstr;
}

wchar_t* C2W(const char* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    int len = ::MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);

    wchar_t *wstr = new wchar_t[len];
    ::ZeroMemory(wstr, len * 2);

    ::MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len);

    return wstr;
}

char* W2C(const wchar_t* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    int len = ::WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, FALSE);

    char *buf = new char[len];
    ::ZeroMemory(buf, len);

    ::WideCharToMultiByte(CP_ACP, 0, str, -1, buf, len, NULL, FALSE);

    return buf;
}


char* W2U(const wchar_t* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    int len = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, FALSE);

    char *buf = new char[len];
    ::ZeroMemory(buf, len);

    ::WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, len, NULL, FALSE);

    return buf;
}

char* C2U(const char* str)
{
    wchar_t *s1 = C2W(str);
    char *s2 = W2U(s1);

    delete s1;
    return s2;
}

/*
char* UnicodeToUtf8(wchar_t* szText)
{
    if (szText == nullptr || szText[0] == L'\0')
        return nullptr;

    int len1 = lstrlenW(szText);
    int len2 = len1 * 3;

    char *szDest = new char[len2 + 1];
    ZeroMemory(szDest, len2 + 1);

    int j = 0;
    for (int i = 0; i < len1 && j < len2; i++)
    {
        unsigned int c = (unsigned int) szText[i];

        if (c <= 0x7F)
        {
            szDest[j++] = (unsigned char) c;
        }
        else if (c > 0x7FF)
        {
            if ((j + 3) > len2)
                break;

            szDest[j++] = ((c >> 12) | 0xE0);
            szDest[j++] = (((c >> 6) & 0x3F) | 0x80);
            szDest[j++] = ((c & 0x3F) | 0x80);
        }
        else
        {
            if ((j + 2) > len2)
                break;

            szDest[j++] = ((c >> 6) | 0xC0);
            szDest[j++] = ((c & 0x3F) | 0x80);
        }
    }

    return szDest;
}
*/