#ifndef BUFF_OP_H
#define BUFF_OP_H

#include <stdlib.h>
#include <memory.h>


/**
* doc-begin
* @brief         摘要         环形缓存队列 数据以包的形式组织
                        数据包的附加数据大小  m_ext_len 确定  范围  0 - def_max_buff_len 之间
                        单个包最多携带数据由  def_max_buff_len 确定
                        总共的缓冲区的都大小  def_max_buff_len 确定
* @param         参数
* @return        返回值
* @date          日期
* doc-end
*/

#define def_cmd_sync_len    (4)             // 命令的结构是20个字节

#define def_cmd_struct_len  (20)            // 命令的结构是20个字节

#define def_max_buff_len    (1024* 3)    // 总的缓冲区的数据长度

#define def_max_exdat_len   (1024)          // 附加缓冲区的数据长度 这个代表着数据分片的长度
                                            // 这个长度只在网络传输中有限制 必须比MTU小 在1550之下


//===================================================> 命令与 回复 列表

//========== define ======== a =========== cmd ==============================//
#ifndef def_cmd_rpc                 // 远程执行一个命令 远程发送的结果 可能会 超过附加数据的长度

#define def_cmd_rpc                 0x0001 // 远程 执行的 主命令
#define def_cmd_rpc_result          0x8001 // 远程 执行的 主命令 的结果 [回复] def_cmd_rpc_sub_on_OB def_cmd_rpc_sub_on_MB

#define def_cmd_rpc_sub_on_OB       0x0001 // 远程 执行的 子命令 在操作箱上执行    [有回复 ] def_cmd_rpc_result
#define def_cmd_rpc_sub_on_MB       0x0002 // 远程 执行的 子命令 在主板上执行      [有回复 ] def_cmd_rpc_result

#endif

//========== define ======== a =========== cmd ==============================//
#ifndef def_cmd_send_file           // 本地 主动发送一个文件

#define def_cmd_send_file           0x0002 // 本地 执行的 主命令 发送文件命令
#define def_cmd_send_file_result    0x8002 // 远端 执行的 主命令 [回复] def_cmd_send_file_get_result  以附加数据的形式回复

#define def_cmd_send_file_head      0x0001 // 本地 执行的 子命令 发送文件头  [无回复 ]
#define def_cmd_send_file_dat       0x0002 // 本地 执行的 子命令 发送文件数据 [无回复 ]
#define def_cmd_send_file_get_result    0x0003 // 本地 执行的 子命令 发送文件数据 [ 有回复 def_cmd_send_file_result ]

#endif

//========== define ======== a =========== cmd ==============================//
#ifndef def_cmd_remote_send_file           // 远端 主动发送一个文件

#define def_cmd_remote_send_file           0x0003 // 远端 执行的 主命令 发送文件命令
#define def_cmd_remote_send_file_result    0x8003 // 本地 执行的 主命令 [回复] def_cmd_send_file_get_result  以附加数据的形式回复

#define def_cmd_remote_send_file_head      0x0001 // 远端 执行的 子命令 发送文件头  [无回复 ]
#define def_cmd_remote_send_file_dat       0x0002 // 远端 执行的 子命令 发送文件数据 [无回复 ]
#define def_cmd_remote_send_file_get_result    0x0003 // 远端 执行的 子命令 发送文件数据 [ 有回复 def_cmd_send_file_result ]

#endif

//========== define ======== a =========== cmd ==============================//
#ifndef def_cmd_host_get_stat           // 本地 主动发送一个获取状态

#define def_cmd_host_get_stat           0x0004 // 本地  获取 状态
#define def_cmd_host_get_stat_replay    0x8004 // 远端  发送 状态

#define def_subcmd_mac_stat             0x0001 // 本地 执行的 子命令 发送状态  [ 回复 ]

#endif

//========== define ======== a =========== cmd ==============================//
#ifndef def_cmd_mac_get_stat           // 远端 主动发送一个获取状态

#define def_cmd_mac_get_stat            0x0005 // 远端  获取 状态
#define def_cmd_mac_get_stat_replay     0x8005 // 本地  发送 状态

#define def_subcmd_host_stat            0x0001 // 远端 执行的 子命令 发送状态  [ 回复 ]

#endif

// 命令与 回复 列表<===================================================




// 数据包的定义 数据包的大小是可变的
struct str_packet
{
    // 0
    unsigned char  m_sync [4]   ;   // 数据的头部

    // -----------------------> 计算crc的起始位置

    // 4
    unsigned short m_main_cmd   ;   // 主命令
    // 6
    unsigned short m_sub_cmd    ;   // 子命令
    // 8
    unsigned short m_para_1     ;   // 参数1
    // 10
    unsigned short m_para_2     ;   // 参数2
    // 12
    unsigned short m_packet_idx ;   // 发送数据包的序号
    // 14
    unsigned short m_ext_len    ;   // 发送数据包的附加数据
    // 16
    unsigned short m_ext_crc    ;   // 发送数据包的附加数据 的 crc

    // <----------------------- 计算crc的结束位置

    // 18
    unsigned short m_packet_crc ;   // 计算

    // 20
    unsigned char  m_ext_dat[def_max_exdat_len];

    // def_max_exdat_len 1024 + 20 ==> 1044
};
//

// 环形缓冲区的结构
struct str_buff_manager
{
    int  m_head_idx ;   // 数据头部的 位置 索引
    int  m_tail_idx ;   // 数据尾部的 位置 索引
    int  m_curr_len ;   // 这个环形缓 冲区 的占用长度
    unsigned char buff[ def_max_buff_len ]; // 缓冲区
};

/**
* doc-begin
* @brief         摘 要 :[   ] op_buff 在 被操纵的缓冲区  中加入一个数据包
* @param         参 数 :[ 1 ] 指针 这个指针是 环形缓冲区的结构体指针
* @param         参 数 :[ 2 ] 指针 这个指针是 插入数据的起始位置
* @param         参 数 :[ 3 ] 整数 这个整数是 插入数据的长度
* @return        返 回 :[   ] 当前 环形数据的 长度 // -1 op_buff == NULL
* doc-end
*/
int  fun_op_buff_add_packet( str_buff_manager * op_buff  , char * add_packet , int size ) ;

/**
* doc-begin
* @brief         摘 要 :[   ] op_buff 在 被操纵的缓冲区  中取得一个数据包
* @param         参 数 :[ 1 ] 指针 这个指针是 环形缓冲区的结构体指针
* @param         参 数 :[ 2 ] 指针 这个指针是 在外部初始化的数据机构体指针 出入之后被 fun_op_buff_get_packet所操作
* @return        返 回 :[   ] 0 成功 //  -1 op_buff == NULL  // -2 get_packet == NULL  // -3 未知错误  //
* doc-end
*/
int  fun_op_buff_get_packet( str_buff_manager * op_buff  , char * get_packet ) ;

/**
* doc-begin
* @brief         摘 要 :[   ] 计算 op_buff 被操纵的缓冲区 中有多少 str_packet 结构的数据包
* @param         参 数 :[ 1 ] 指针 这个指针是 环形缓冲区的结构体指针
* @return        返 回 :[   ] 数据包的个数 // -1 op_buff == NULL
* doc-end
*/
int  fun_op_buff_get_packet_count( str_buff_manager * op_buff) ;

/**
* doc-begin
* @brief         摘 要 :[   ] 计算 packet 的拓展数据的crc 包头的crc
* @param         参 数 :[ 1 ] 指针 这个指针是 需要被打包的包
* @param         参 数 :[ 2 ] 指针 这个指针是 附加数据 的 指针
* @param         参 数 :[ 3 ] 整数 这个整数是 附加数据 的 长度
* @return        返 回 :[   ] 数据包的个数 // -1 packet == NULL
* doc-end
*/

int  fun_op_packing( str_packet * packet , char * exdat , int exdat_len) ;


/**
* doc-begin
* @brief         摘 要 :[   ] 计算  crc
* @param         参 数 :[ 1 ] 指针 这个指针是 需要被计算的crc 数据缓冲区
* @param         参 数 :[ 2 ] 整数 这个整数是 需要被计算的crc 缓冲区的长度
* @return        返 回 :[   ] 缓冲区的CRC
* doc-end
*/

unsigned short fun_op_calc_crc16(char *  dat  , int dat_len) ;

/**
* doc-begin
* @brief         摘 要 :[   ] 获取缓冲区的一个字符
* @param         参 数 :[ 1 ] 整数 这个整数是 需要 被 取得 的 索引
* @return        返 回 :[   ] 一个字符
* doc-end
*/

unsigned char fun_op_get_one_byte( str_buff_manager * op_buff , int idx );




#endif // BUFF_OP_H
