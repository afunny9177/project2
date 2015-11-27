/*!
 * \file utf8.h
 *
 * \author Administrator
 * \date 五月 2014
 *
 * 用于ANSI字符串和UNICODE字符串以及UTF-8字符串的相互转换！！
 */

#ifndef __UTF8_H__
#define __UTF8_H__


/////////////////////////////////////////////////////////////////////////////////////
// 通用函数！

/**
* @brief 多字节字符串转为宽字符字符串（936代码页，非中文系统下用）
* @param [in] str 多字节字符串
* @return 宽字节字符串
* @note
* 释放返回指针的内存
*/
wchar_t* C2W936(const char* str);

/**
 * @brief 多字节字符串转为宽字符字符串
 * @param [in] str 多字节字符串
 * @return 宽字节字符串
 * @note
 * 释放返回指针的内存
 */
wchar_t* C2W(const char* str);

/**
 * @brief 宽字符串转为多字节字符串
 * @param [in] str 宽字节字符串
 * @return 多字节字符串
 * 释放返回指针的内存
 */
char* W2C(const wchar_t* str);

/**
 * @brief 宽字节字符串转换UTF-8字符串
 * @param [in] str 宽字节串
 * @return UTF-8格式字符串
 * @note
 * 释放返回指针的内存
 */
char* W2U(const wchar_t* str);

/**
 * @brief 多字节字符串转为UTF-8字符串
 * @param [in] str 多字节字符串
 * @return UTF-8格式字符串
 * @note
 * 释放返回指针的内存
 */
char* C2U(const char* str);

// 宽字节字符串转为UTF8字符串的另一种实现，记得要释放！！
//char* UnicodeToUtf8(wchar_t* szText);

#endif