#include "gagent.h"
#include "hal_receive.h"

uint8 *hal_RxBuffer=NULL;
uint32 pos_start = 0, pos_check=0, pos_current = 0;

/****************************************************************
FunctionName    :   get_data_len
Description     :   get data length from "from " to "to"
return          :   the length of data.
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
int32 get_data_len( int32 from, int32 to )
{ 
    if(to >= from)
        return to  - from;
    else
        return BUF_LEN - from + to;
}

/****************************************************************
FunctionName    :   get_available_buf_space
Description     :   get buf of availabel size of buf
pos_current     :   current position  
pos_start       :   start position 
return          :   available size
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
int32 get_available_buf_space(int32 pos_current, int32 pos_start)
{
    if(pos_current >= pos_start)
    {
        return HAL_BUF_SIZE - pos_current; 
    }
    else //pos_current < pos_start
    {
        return pos_start - pos_current;
    }
}
/****************************************************************
FunctionName    :   advance_pos
Description     :   get position after advance 
pos_current     :   current position  
addition0       :   want to advance value. 
return          :   after advance of pos_current .
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
int32 advance_pos( int32 pos_current, int32 addition0 )
{
    int32 addition = addition0 % HAL_BUF_SIZE;

    if(addition >= 0)
    {
        if( pos_current + addition < HAL_BUF_SIZE)
        {
            return pos_current + addition;
        }  
        else if( pos_current + addition == HAL_BUF_SIZE)
        {
            return 0;
        }  
        else //current_pos + addition > BUF_LEN
        {
            return addition - (HAL_BUF_SIZE - pos_current);
        }
    }
    else // addition < 0
    {
        if( pos_current + addition >= 0)
        {
            return pos_current + addition; 
        }
        else // current_pos + addition < 0
        {
            return HAL_BUF_SIZE - (-addition-pos_current );
        }
    }
}
/****************************************************************
FunctionName    :   move_data_backward
Description     :   move data backward 
buf             :   the pointer of hal_buf   
pos_start       :   the pos of start of move. 
pos_end         :   the pos of end of move.
move_len        :   the  length of data need to move.
return          :   NULL
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
// 向前（左）移动数据，被移动的数据为从p_start_pos开始，到p_end_pos，移动长度为move_len
void move_data_backward( uint8 *buf, int32 pos_start, int32 pos_end, int32 move_len)
{
    int32 k;
    int32 pos_new_start = advance_pos(pos_start, (-1)*move_len);
    int32 move_data_len = get_data_len(pos_start, pos_end);
    
    for(k=0; k < move_data_len; k++)
    {
        buf[advance_pos(pos_new_start, k)] = buf[advance_pos(pos_start, k)];
    }
}

/****************************************************************
FunctionName    :   get_last_pack_head_pos
Description     :   try to find last pack position of head.
buf             :   the pointer of hal_buf   
pos_start       :   the pos of start of move. 
p_check         :   the pos of check.
return          :   -1 can't find the head .
                    other the position of head.
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
int32 get_last_pack_head_pos( uint8 *buf, int32 pos_start, int32 pos_check)
{
    // 找出上一个包的包头位置或起始位置
    // 如果找到上一个包头，返回该包头位置
    // 如果找不到上一个包头，-1
    int32 pos_last_pack_head = pos_check; // 用于存放上一个包头位置

    if(pos_last_pack_head == pos_start || advance_pos(pos_last_pack_head, -1) == pos_start)
    {
        pos_last_pack_head = -1;
    }
    else
    {
        pos_last_pack_head = advance_pos(pos_check, -2);
        while(pos_last_pack_head != pos_start && !(buf[pos_last_pack_head] == 0xFF && buf[advance_pos(pos_last_pack_head, 1)] == 0xFF))
        {
            pos_last_pack_head = advance_pos(pos_last_pack_head, -1);
        }
        if(pos_last_pack_head == pos_start && !(buf[pos_last_pack_head] == 0xFF && buf[advance_pos(pos_last_pack_head, 1)] == 0xFF))
        {
            pos_last_pack_head = -1;
        }
    }

    return pos_last_pack_head;
}

/****************************************************************
FunctionName    :   hal_ReceivepOnePack
Description     :   try to receive one pack data from local.
fd              :   local fd. 
buf             :   the pointer of hal_buf   
return          :   0 no data from local 
                    >0 the local packet data length.
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
int32  hal_ReceivepOnePack( int32 fd,uint8 *buf )
{
    int32 available_len =0;
    int32 recved_data_len = 0;
    int32 read_count=0;
    int32 check_data_len=0;
    int32 i=0,j=0;

    /* step 1.read data into loopbuf */
    available_len = get_available_buf_space( pos_current, pos_start );
    read_count = serial_read( fd, &hal_RxBuffer[pos_current],available_len );
    pos_current = advance_pos( pos_current,read_count );

    /* delete 0x55 after 0xff ,and will change p_current value */
    check_data_len = get_data_len( pos_check,pos_current );
    while( check_data_len>=2 )
    {
        if( hal_RxBuffer[pos_check] != 0xFF )
        {
        pos_check = advance_pos( pos_check,1 );
        check_data_len = get_data_len(pos_check, pos_current);
        }
        else
        {
            // 2.1 如0xFF后为0x55则剔除0x55
            if(hal_RxBuffer[advance_pos(pos_check, 1)] == 0x55)
            {
                // 把0x55后的数据整体前移1个B
                move_data_backward( hal_RxBuffer, advance_pos(pos_check, 2), pos_current, 1);

                pos_current = advance_pos(pos_current, -1);
                pos_check = advance_pos(pos_check, 1);
                check_data_len = get_data_len(pos_check, pos_current);
            }
            // 2.2 如0xFF后为0xFF表示出现了新的数据包，则判断前一数据包是否为一完整数据包，否则去除该不完整的包
            else if(hal_RxBuffer[advance_pos(pos_check, 1)] == 0xFF)
            {
                // 2.2.1 找出上一个包头的位置
                //GAgent_Printf( GAGENT_INFO,"pos_start=%d pos_check=%d",pos_start,pos_check );
                int32 pos_last_pack_head = get_last_pack_head_pos(hal_RxBuffer, pos_start, pos_check);
                if(pos_last_pack_head == -1) // 没有找到了上一个包头
                {
                    //GAgent_Printf( GAGENT_INFO,"Can't found pos_last_pack_head pos_check=%d",pos_check);
                    pos_check = advance_pos(pos_check, 1);
                    check_data_len = get_data_len(pos_check, pos_current);
                }
                else // 找到了上一个包头
                {
                    int8 is_invalid_pack = 0;

                    int32 last_pack_len = get_data_len(pos_last_pack_head, pos_check);
                    if(last_pack_len < 4)
                    {
                        is_invalid_pack = 1;
                    }
                    else if((hal_RxBuffer[advance_pos(pos_last_pack_head, 2)]*256 + hal_RxBuffer[advance_pos(pos_last_pack_head, 3)]) + 4 != last_pack_len)
                    {
                        is_invalid_pack = 1;
                    }
                    else // valid len
                    {
                        is_invalid_pack = 0;
                    }

                    // 2.2.2 踢除上一个不完整的包(把后面的数据整体前移)
                    if (is_invalid_pack)
                    {
                        int32 move_len = last_pack_len;
                        move_data_backward(hal_RxBuffer, pos_check, pos_current, move_len);

                        pos_current = advance_pos(pos_current, (-1)*move_len);
                        pos_check = advance_pos(pos_last_pack_head, 1);
                        check_data_len = get_data_len(pos_check, pos_current);
                    }
                    else
                    {
                        pos_check = advance_pos(pos_check, 1);
                        check_data_len = get_data_len(pos_check, pos_current);
                    }
                }                
            }
            else
            {
                /* 0XFF 后面不是0XFF 也不是0X55 */
                // 2.3.1 找出上一个包头的位置
                int32 pos_last_pack_head = get_last_pack_head_pos(hal_RxBuffer, pos_start, pos_check);
                if(pos_last_pack_head == -1) // 没有找到了上一个包头
                {
                    pos_check = advance_pos(pos_check, 1);
                    check_data_len = get_data_len(pos_check, pos_current);
                }
                else // 找到了上一个包头
                {
                    // 2.3.2 踢除上一个不完整的包(把后面的数据整体前移)
                    int32 move_len = get_data_len(pos_last_pack_head, pos_check);
                    move_data_backward(hal_RxBuffer, pos_check, pos_current, move_len);

                    pos_current = advance_pos(pos_current, (-1)*move_len);
                    pos_check = advance_pos(pos_last_pack_head, 1);
                    check_data_len = get_data_len(pos_check, pos_current);
                }

            }

        }
    }
    
    /* step3.find the head of local data */
    recved_data_len = get_data_len(pos_start, pos_current);
    if( recved_data_len<4 )
        return 0;
    while(recved_data_len >= 4 && (
                !(hal_RxBuffer[pos_start] == 0xFF && hal_RxBuffer[advance_pos(pos_start, 1)] == 0xFF) ||
                hal_RxBuffer[advance_pos(pos_start, 2)] * 256 + hal_RxBuffer[advance_pos(pos_start, 3)] + 4 >= HAL_BUF_SIZE)
         )
    {
        pos_start = advance_pos(pos_start, 1);
        recved_data_len = get_data_len(pos_start, pos_current);
    }
    /* step4 deal data */
    while ( recved_data_len>=4 )
    {
        int32 pack_len=0;
        int32 pos_replaced = 0;

        pack_len = hal_RxBuffer[advance_pos(pos_start,2)]*256 + hal_RxBuffer[ advance_pos(pos_start,3)];
        if( recved_data_len >= (pack_len+4) )/* one package already */
        {
            int32 i=0;
            /* copy data to user application */
            for( i=0;i<(pack_len+4);i++ )
            {
                buf[i] = hal_RxBuffer[advance_pos(pos_start,i)];
            }
            pos_start = advance_pos( pos_start,pack_len+4 );
            recved_data_len = get_data_len( pos_current,pos_start );
            return (pack_len+4);
        }
        else
        {
            return 0;
        }
    }
    
    return 0;
}
void hal_ReceiveInit(  )
{   
    hal_RxBuffer = (uint8*)malloc( HAL_BUF_SIZE );
    while( hal_RxBuffer==NULL )
    {
        hal_RxBuffer = (uint8*)malloc( HAL_BUF_SIZE );
        sleep(1);
    }
}