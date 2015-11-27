/*!
 * \file pluginapi.h
 *
 * \author Administrator
 * \date 七月 2014
 *
 * NSIS插件API头文件，用于插件的操作等
 */

#ifndef __PLUGINAPI_H__
#define __PLUGINAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NSISCALL
#define NSISCALL __stdcall
#endif

////////////////////////////// 前向声明 /////////////////////////////////

struct _stack_t;                            // 参数（字符串）堆栈
struct _extra_parameters;                   // 额外参数（回调函数指针）

/////////////////////////////////////////////////////////////////////////


////////////////////////////// 全局变量 /////////////////////////////////

extern unsigned int g_stringsize;           // 参数（字符串类型）的长度
extern struct _stack_t **g_stacktop;        // 指向栈顶的二级指针，*g_stacktop为g_st
extern TCHAR *g_variables;                  // 指向用户变量的指针($0、$1、$2...)
extern struct _extra_parameters *g_extra;   // 额外参数，可以执行几个回调函数

/////////////////////////////////////////////////////////////////////////


////////////////////////////////////////// 宏定义 //////////////////////////////////////////////////

// 插件初始化宏，一般在第一个插件函数内使用，方便调用
#define NSIS_PLUGIN_INIT()          \
do {                                \
    g_stringsize = string_size;     \
    g_variables = variables;        \
    g_stacktop = stacktop;          \
    g_extra = extra;                \
} while (0);

// 从NSIS 2.42版本开始，你可以用插件API（exec_flags->plugin_api_version）检测NSIS版本号，格式是0xXXXXYYYY，X为主版本号，Y为次版本
// 号（MAKELONG(X,Y)），当做版本检测时，一定要记得用>=，例如if (pX->exec_flags->plugin_api_version >= NSISPIAPIVER_1_0) {}
#define NSISPIAPIVER_1_0        0x00010000          // 1.0版本
#define NSISPIAPIVER_CURR       NSISPIAPIVER_1_0    // 当前版本

// 定义给页面显示插件用，为更好理解、使用请看源码：Ui.c
// 当点击按钮时（产生WM_COMMAND消息，父窗口$HWNDPARENT会收到），NSIS会发送消息SendMessage(g_hwnd,WM_NOTIFY_OUTER_NEXT,(WPARAM)delta,0)
// 给父窗口，wParam的值主要有3个（1、-1、'x'/NOTIFY_BYE_BYE），父窗口收到wParam为1或-1时会将当前子对话框销毁，然后再用CreateDialogParam
// 创建一个新的子对话框，这样便实现了上一步、下一步的功能了（上一步的wParam值为-1，下一步的wParam值为1，NSIS中的this_page加上此值）
// 不管你的安装程序有几个页面，顺序如何
#define WM_NOTIFY_OUTER_NEXT    (WM_USER+0x8)

// 自定义页面要发送这个消息给$HWNDPARENT，让NSIS知道自定义页面已经准备好了
#define WM_NOTIFY_CUSTOM_READY  (WM_USER+0xd)

// 当用户取消安装时作为发送WM_NOTIFY_OUTER_NEXT消息的wParam参数——当心它的警告
#define NOTIFY_BYE_BYE          'x'     // 退出安装包
#define NOTIFY_NEXT              1      // 到下一页面
#define NOTIFY_BACK             -1      // 到上一页面

// 控件ID号
#define IDBACK                   3      // 上一步按钮的ID号


/////////////////////////// 检举类型和插件回调函数声明 /////////////////////////

typedef enum _user_variables
{
    // 可读，可写
    INST_0,             // $0
    INST_1,             // $1
    INST_2,             // $2
    INST_3,             // $3
    INST_4,             // $4
    INST_5,             // $5
    INST_6,             // $6
    INST_7,             // $7
    INST_8,             // $8
    INST_9,             // $9
    INST_R0,            // $R0
    INST_R1,            // $R1
    INST_R2,            // $R2
    INST_R3,            // $R3
    INST_R4,            // $R4
    INST_R5,            // $R5
    INST_R6,            // $R6
    INST_R7,            // $R7
    INST_R8,            // $R8
    INST_R9,            // $R9
    INST_CMDLINE,       // $CMDLINE
    INST_INSTDIR,       // $INSTDIR
    INST_OUTDIR,        // $OUTDIR
    INST_EXEDIR,        // $EXEDIR
    INST_LANGUAGE,      // $LANGUAGE

    // 只读
    INST_TEMP,          // $TEMP
    INST_PLUGINSDIR,    // $PLUGINSDIR
    INST_EXEPATH,       // $EXEPATH
    INST_EXEFILE,       // $EXEFILE
    INST_RESERVE,       // None
    INST_CLICKNEXT,     // next button text

    // 结束
    __INST_LAST         // end
} user_variables;

// NSIS插件回调消息
typedef enum _NSPIM 
{
    NSPIM_UNLOAD,       // 这是插件能收到的最后一条消息，可做最后的清理工作
    NSPIM_GUIUNLOAD,    // 在.onGUIEnd之后发送这个消息到插件回调函数
} NSPIM;

// 插件回调函数的原型，使用extra_parameters->RegisterPluginCallback()注册
// 未知的消息返回NULL，为了便于扩展应总是声明为__cdecl的调用约定
typedef UINT_PTR (*NSISPLUGINCALLBACK)(NSPIM);

///////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////// 结构体定义 //////////////////////////////////////////


//  NSIS参数堆栈，是用一个链表实现的，链表的结点就是这个_stack_t结构结构中有两个成员，第一个成
//  员是指向下一个节点的指针，第二个成员是参数的值，例如插件命令plugin::function_name $HWNDPARENT "plugin"
//  NSIS解析时会将参数从右到左依次压入堆栈，并调整stack_t *g_st全局堆栈指针变量指向栈顶，NSIS解
//  析参数时不管参数的类型，而是全部视为字符串类型，你可以根据需要将字符串转换成想要的类型。NSIS
//  中给插件传递的参数都是g_hwnd,NSIS_MAX_STRLEN,(char*)g_usrvars,(void*)&g_st,&plugin_extra_parameters
//  一般调用插件中的第一个函数时会把其中几个参数保存在插件的全局变量中如g_stacktop是一个指向g_st
//  的二级指针，当g_st的指向改变时，g_stacktop还是指向g_st，因此可以说g_stacktop是一个指向栈顶的
//  二级指针，如果是一级的，则当g_st改变时，g_stacktop不会改变，就拿不到栈顶的参数！
typedef struct _stack_t
{
    struct _stack_t *next;      // 指向下一个堆栈结构的指针
    TCHAR text[1];              // 参数（字符串长度为string_size）
} stack_t;

// extra_parameters结构的exec_flags成员包含了其它有趣的信息（除了堆栈、变量和句柄）
typedef struct _exec_flags_t
{
    int autoclose;              // 是否自动关闭（SetAutoClose），0为假，非0为真
    int all_user_var;           // 是否所有用户（SetShellVarContext），0为假，非0为真
    int exec_error;             // 错误标志
    int abort;                  // 是否终止安装
    int exec_reboot;            // 重启标志（编译要打开NSIS_SUPPORT_REBOOT）
    int reboot_called;          // 取消重启（编译要打开NSIS_SUPPORT_REBOOT）
    int XXX_cur_insttype;       // 已过时
    int plugin_api_version;     // NSIS版本号
    int silent;                 // 是否静默安装（编译要打开NSIS_CONFIG_SILENT_SUPPORT）
    int instdir_error;          // 安装目录有错？
    int rtl;                    // 从右到左显示文字？
    int errlvl;                 // 错误级别
    int alter_reg_view;         // 和注册表有关
    int status_update;          // 状态更新有关（显示安装包前那个解压小窗口）
} exec_flags_t;



//  额外参数，为NSIS插件中导出函数中的最后一个参数的值，你可以用此结构声明一个变量，然
//  后将最后一个参数的值保存在这个变量中，以便后续调用ExecuteCodeSegment等做准备！！！
typedef struct _extra_parameters
{
    // 指向exec_flags_t的指针，保存有plugin_api_version等值
    exec_flags_t *exec_flags;
    // 执行NSIS脚本中定义的函数，第一个参数表示NSIS函数地址，通过GetFunctionAddress得到
    // 通常在调用时将第一个参数减1，第二个参数通常指定为NULL！！
    int (NSISCALL *ExecuteCodeSegment)(int, HWND);
    void (NSISCALL *validate_filename)(TCHAR *);
    // 返回0表示注册插件回调成功，返回1表示之前已经注册过，小于0表示有错误
    int (NSISCALL *RegisterPluginCallback)(HMODULE, NSISPLUGINCALLBACK); 
} extra_parameters;

///////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////// 函数声明 ///////////////////////////////////////


//  从参数堆栈中弹出一个参数（字符串类型），并拷贝到str缓冲区中，str必须已经分配好内存
//  返回0表示执行成功，返回1表示堆栈为空，取不到值，在NSIS中参数都是字符串，自己转换了
int NSISCALL popstring(TCHAR *str); 

//  从参数堆栈中弹出一个参数（字符串类型），和popstring一样，只是多了一个参数maxlen，它
//  指定最多复制maxlen个字节（包括\0）到str缓冲区中，如果传递给maxlen是0，表示没有限制
int NSISCALL popstringn(TCHAR *str, int maxlen);  

//  从参数堆栈中弹出一个参数（布尔类型），当然内部会比较是不是字符串“true”等来返回真假	
bool NSISCALL popbool();

//  从参数堆栈中弹出一个参数，返回值为将参数字符串转换后的整型值，获取整型参数时经常用到
int NSISCALL popint();  

//  从参数堆栈中弹出一个参数，和popint一样，只是支持参数中有或（|）这样的参数！！
int NSISCALL popint_or(); 

//  转换字符串类型到整型，工具函数，一般内部使用
int NSISCALL myatoi(const TCHAR *s); 

//  转换字符串类型到无符号类型，工具函数，一般内部使用
unsigned NSISCALL myatou(const TCHAR *s);

//  转换字符串类型到整型，和myatoi一样，只是支持或（|）这样的参数字符串
int NSISCALL myatoi_or(const TCHAR *s);

//  向参数堆栈中压入一个参数（字符串），会更改g_st指向压入后的参数（在栈顶），但是g_stacktop
//  始终指向g_st，也就是直接指向栈顶，这就是二级指针的好处！！！
void NSISCALL pushstring(const TCHAR *str);

//  向参数堆栈中压入一个参数（整型），当然内部会转换成字符串后再压入参数堆栈中！！！
void NSISCALL pushint(int value);

//  获取用户变量（$0、$1、$2...）的值，传给varnum的值应为INST_0..INST_LANG这些枚举值
TCHAR* NSISCALL getuservariable(const int varnum);

// 设置用户变量（$0、$1、$2...）的值，传给varnum的值应为INST_0..INST_LANG这些枚举值
void NSISCALL setuservariable(const int varnum, const TCHAR *var);

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // !__NSIS_H__
