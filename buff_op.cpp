#include "buff_op.h"

// 环形缓冲区的插入的时候要分两段插入
int  fun_op_buff_add_packet( str_buff_manager * op_buff  , char * add_packet , int size )
{
    int ret_val = 0 ;

    if( op_buff == NULL )
    {
        ret_val = -1 ;
        return  ret_val ;
    }

    // 如果要插入的数据 不能一下写进缓冲区就 分量两块去写
    if( op_buff->m_tail_idx + size > def_max_buff_len )
    {


        int size1 = def_max_buff_len - op_buff->m_tail_idx  ;
        int size2 = size - size1                            ;

        memcpy( op_buff->buff + op_buff->m_tail_idx , add_packet , size1 ) ;
        memcpy( op_buff->buff   , add_packet + size1  , size2 ) ;

        op_buff->m_curr_len = def_max_buff_len ;

        op_buff->m_tail_idx = (  op_buff->m_tail_idx + size  ) % def_max_buff_len;


        // 如果剩余的数据区不足以存放要存的数据就 覆盖掉头部的数据
        if( size >  (  def_max_buff_len -   op_buff->m_curr_len ) )
        {
            op_buff->m_head_idx = op_buff->m_tail_idx + 1 ;
        }

    }
    else
    {
        memcpy( op_buff->buff + op_buff->m_tail_idx , add_packet , size )  ;

        op_buff->m_curr_len = op_buff->m_curr_len + size   ;
        op_buff->m_tail_idx = (  op_buff->m_tail_idx + size  ) % def_max_buff_len;

        // 如果剩余的数据区不足以存放要存的数据就 覆盖掉头部的数据
        if( size >  (  def_max_buff_len -   op_buff->m_curr_len ) )
        {
            op_buff->m_head_idx = op_buff->m_tail_idx + 1 ;

            op_buff->m_curr_len = def_max_buff_len ;
        }
    }
    ret_val = op_buff->m_curr_len ;
    return  ret_val ;
}

int  fun_op_buff_get_packet( str_buff_manager * op_buff  , char * get_packet )
{
    int ret_val = 0 ;

    if( op_buff == NULL )
    {
        ret_val = -1 ;
        return  ret_val ;
    }

    if( get_packet == NULL )
    {
        ret_val = -2 ;
        return  ret_val ;
    }

    char temp_buff[4];

    memset( temp_buff , 0 , 4);

    for (int var = 0; var < op_buff->m_curr_len-4; )
    {
        // 读取前四个字符
        for (int var_1 = 0; var_1 < 4; ++var_1)
        {
            temp_buff[var_1] = fun_op_get_one_byte( op_buff  , var + var_1 ) ;
        }

        if(
                ( temp_buff [ 0 ] == 's')&&
                ( temp_buff [ 1 ] == 'y')&&
                ( temp_buff [ 2 ] == 'n')&&
                ( temp_buff [ 3 ] == 'c')
                )
        {
            ret_val ++ ; // 存在的数据包加1

            // 读取前四个字符
            for (int var_1 = 0; var_1 < def_cmd_struct_len; ++var_1)
            {
                unsigned char dat  = fun_op_get_one_byte( op_buff  , var + var_1 ) ;
                memcpy( get_packet + var_1 , &dat , 1 ) ;
            }

            int extdat_len = ((str_packet * )get_packet)->m_ext_len ; // 这个包的附加数据长度

            // 读取附加数据
            for (int var_1 = 0; var_1 < extdat_len; ++var_1)
            {
                unsigned char dat  = fun_op_get_one_byte( op_buff  , var + var_1 + def_cmd_struct_len ) ;
                memcpy( get_packet + def_cmd_struct_len + var_1 , &dat , 1 ) ;
            }

            if( ( op_buff->m_head_idx + def_cmd_struct_len + extdat_len ) > def_max_buff_len )
            {
                op_buff->m_head_idx =  (op_buff->m_head_idx + def_cmd_struct_len + extdat_len) % def_max_buff_len;
            }
            else
            {
                op_buff->m_head_idx =  op_buff->m_head_idx + def_cmd_struct_len + extdat_len ;
            }

            op_buff->m_curr_len =   op_buff->m_curr_len - (def_cmd_struct_len + extdat_len);

            break ;
        }
        else
        {
            ++var ; // 没找到 数据包 索引向前推进一个字节长度
        }
    }




    return  ret_val ;
}

int  fun_op_buff_get_packet_count( str_buff_manager * op_buff)
{
    int ret_val = 0 ;

    if( op_buff == NULL )
    {
        ret_val = -1 ;
        return  ret_val ;
    }

    str_packet i_op_pack;
    memset( & i_op_pack , 0 , sizeof( str_packet ) ) ;

    char temp_buff[4];

    memset( temp_buff , 0 , 4);

    int zhenshi =  0 ;
    for (int var = 0; var < op_buff->m_curr_len-4; )
    {

         zhenshi =  op_buff->m_head_idx + var ;
        // 读取前四个字符
        for (int var_1 = 0; var_1 < 4; ++var_1)
        {
            temp_buff[var_1] = fun_op_get_one_byte( op_buff  , var + var_1 ) ;
        }

        if(
                ( temp_buff [ 0 ] == 's')&&
                ( temp_buff [ 1 ] == 'y')&&
                ( temp_buff [ 2 ] == 'n')&&
                ( temp_buff [ 3 ] == 'c')
                )
        {
            ret_val ++ ; // 存在的数据包加1

            // 读取前四个字符
            for (int var_1 = 0; var_1 < def_cmd_struct_len; ++var_1)
            {
                unsigned char dat  = fun_op_get_one_byte( op_buff  , var + var_1 ) ;
                memcpy( (char * ) (& i_op_pack ) + var_1 , &dat , 1 ) ;
            }

            int extdat_len = i_op_pack.m_ext_len ; // 这个包的附加数据长度

            // 查找下一个包要跳过的长度 是 str_packet 长度 def_cmd_struct_len +  extdat_len ;
            int jump_len =  def_cmd_struct_len +  extdat_len ;

            var = var + jump_len ; //找到 数据包 索引向前推进一个数据包长度 ( 包括数据包的附加长度 ）
        }
        else
        {
            ++var ; // 没找到 数据包 索引向前推进一个字节长度
        }
    }
    return  ret_val ;
}

unsigned char fun_op_get_one_byte( str_buff_manager * op_buff , int idx )
{
    unsigned char dat ;

    int zhenshi_idx = 0 ; //真实的索引

    if( op_buff->m_head_idx + idx > def_max_buff_len )
    {
        zhenshi_idx = (  op_buff->m_head_idx + idx ) %  def_max_buff_len;
    }
    else
    {
        zhenshi_idx = op_buff->m_head_idx + idx ;
    }

    dat = op_buff->buff [ zhenshi_idx ] ;

    return dat ;
}

int  fun_op_packing( str_packet * packet , char * exdat , int exdat_len)
{
    int ret_val = 0 ;

    if( packet == NULL )
    {
        ret_val = -1 ;
        return  ret_val ;
    }

    char sync_buff[4];
    memset( sync_buff , 0   , def_cmd_sync_len );

    sync_buff[0] = 's';
    sync_buff[1] = 'y';
    sync_buff[2] = 'n';
    sync_buff[3] = 'c';

    memcpy( packet->m_sync , sync_buff , def_cmd_sync_len );

    packet->m_ext_len = exdat_len;
    packet->m_ext_crc = fun_op_calc_crc16( exdat , exdat_len );

    packet->m_packet_crc = fun_op_calc_crc16( ((char * ) &packet ) + def_cmd_sync_len  , exdat_len );

    memcpy( packet->m_ext_dat , exdat , exdat_len );


    return  ret_val ;
}

unsigned short fun_op_calc_crc16(char *  dat  , int dat_len)
{

    return 0 ;
}

