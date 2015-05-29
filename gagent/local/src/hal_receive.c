#include "gagent.h"
#include "hal_receive.h"

uint8 *hal_RxBuffer=NULL;
/*
 pos_start:pos of get
 pos_current:pos of put
 */
uint32 pos_start = 0, pos_current = 0;

static uint8 __halbuf_read(uint32 offset)
{
    return hal_RxBuffer[offset & HAL_BUF_MASK];
}

static void __halbuf_write(uint32 offset, uint8 value)
{
    hal_RxBuffer[offset & HAL_BUF_MASK] = value;
}
/****************************************************************
FunctionName    :   get_data_len
Description     :   get data length from "from " to "to"
return          :   the length of data.
Add by Alex.lin and Johnson     --2015-04-07
****************************************************************/
static int32 get_data_len( int32 from, int32 to )
{ 
    to = to & HAL_BUF_MASK;
    from = from & HAL_BUF_MASK;
    
    if(to >= from)
        return to  - from;
    else
        return HAL_BUF_SIZE - from + to;
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
    pos_current = pos_current & HAL_BUF_MASK;
    pos_start = pos_start & HAL_BUF_MASK;
    
    if(pos_current >= pos_start)
    {
        return HAL_BUF_SIZE - pos_current;
    }
    else
    {
        return pos_start - pos_current;
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
    int32 pos_new_start;
    int32 move_data_len = get_data_len(pos_start, pos_end);

    pos_new_start = pos_start + (-1)*move_len;
    
    for(k=0; k < move_data_len; k++)
    {
        __halbuf_write(pos_new_start + k, __halbuf_read(pos_start + k));
    }
}

/****************************************************************
FunctionName    :   find_packet_backward
Description     :   在串口缓冲区中，从指定位置往回查找包含该数据的一帧数据，
                    并返回帧头所在偏移。没有完整一帧数据则返回已接收数据的起始位置。
pos             :   指定位置. 
return          :   帧头所在偏移
Add by will     --2015-05-21
****************************************************************/
static int32 find_packet_backward(uint32 pos)
{
    uint32 offset;
    uint8 data1;
    uint8 data2;
    uint32 datalen;
    uint16 packetlen;

    // 缓冲区中已知数据长度
    datalen = get_data_len(pos_start, pos);
    if(datalen < MCU_SYNC_HEAD_LEN)
    {
        return pos_start;
    }

    if(datalen <= MCU_SYNC_HEAD_LEN + sizeof(packetlen))
    {
        return pos_start;
    }
    
    offset = pos - MCU_SYNC_HEAD_LEN;
    while(offset != pos_start)
    {
        data1 = __halbuf_read(offset);
        data2 = __halbuf_read(offset + 1);

        if(MCU_HDR_FF == data1 && MCU_HDR_FF == data2)
        {
            packetlen = __halbuf_read(offset + MCU_LEN_POS);
            packetlen = (packetlen << 8) | __halbuf_read(offset + MCU_LEN_POS + 1);
            if(datalen <= packetlen)
            {
                return offset;
            }
            else
            {
                return offset + packetlen;
            }
            break;
        }
        offset--;
    }

    return pos_start;
}

/****************************************************************
FunctionName    :   find_packet_forward
Description     :   在串口缓冲区中，从指定位置往前查找最近一帧完整数据，
                    返回该帧数据头所在位置。
pos             :   指定位置. 
packetLen       :   输出参数，该帧数据的长度
return          :   失败--RET_FAILED
                    成功--帧头所在偏移
Add by will     --2015-05-21
****************************************************************/
static int32 find_packet_forward(uint32 pos, uint16 *packetLen)
{
    uint8 data1;
    uint8 data2;
    uint32 datalen;
    uint16 packetlen;
    uint32 start, end;

    start = pos;
    end = pos_current;
    datalen = get_data_len(start, end);
    if(datalen < MCU_SYNC_HEAD_LEN)
    {
        return RET_FAILED;
    }

    if(datalen <= MCU_SYNC_HEAD_LEN + sizeof(packetlen))
    {
        return RET_FAILED;
    }
    
    while(start != end)
    {
        data1 = __halbuf_read(start);
        data2 = __halbuf_read(start + 1);

        if(MCU_HDR_FF == data1 && MCU_HDR_FF == data2)
        {
            packetlen = __halbuf_read(start + MCU_LEN_POS);
            packetlen = (packetlen << 8) | __halbuf_read(start + MCU_LEN_POS + 1);
            packetlen += 4;
            if(datalen < packetlen)
            {
                return RET_FAILED;
            }
            else
            {
                *packetLen = packetlen;
                return start;
            }
        }
        start++;
    }

    return RET_FAILED;
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
    int32 ret;
    int32 available_len =0;
    int32 read_count;
    int32 i=0;
    static uint8 isSyncHead = 0;
    uint32 offRec;
    uint8 data;
    uint16 packetLen;
    uint32 invalidLen;

    /* step 1.read data into loopbuf */
    available_len = get_available_buf_space( pos_current, pos_start );
    read_count = serial_read( fd, &hal_RxBuffer[pos_current & HAL_BUF_MASK],available_len );
    offRec = pos_current;
    if(read_count <= 0)
    {
        return 0;
    }
    pos_current += read_count;
    for(i = 0; i < read_count; i++)
    {
        data = __halbuf_read(offRec);
        if(isSyncHead)
        {
            isSyncHead = 0;
            /* check if is a new packet */
            if(0xFF == data)
            {
                offRec++;
                continue;
            }
            /* check if is 0xff 0x55 */
            else if(0x55 == data)
            {
                if((offRec & HAL_BUF_MASK) == ((pos_current - 1) & HAL_BUF_MASK))
                {
                    /* the last byte has received */
                    __halbuf_write(offRec, 0x00);
                }
                else
                {
                    move_data_backward(hal_RxBuffer, offRec + 1, pos_current, 1);
                }
                offRec -= 1;
                pos_current -= 1;
            }
            /* invalid packet */
            else
            {
                /* 找到当前字节处于哪一个packet */
                ret = find_packet_backward(offRec);
                invalidLen = get_data_len(ret, offRec + 1);
                if((offRec & HAL_BUF_MASK) == ((pos_current - 1) & HAL_BUF_MASK))
                {
                    /* the last byte has received,
                     * relocate to the end of pre packet
                     */
                    offRec = ret;
                }
                else
                {
                    move_data_backward(hal_RxBuffer, offRec + 1, pos_current, invalidLen);
                }
                offRec -= invalidLen;
                pos_current = pos_current - invalidLen;
            }
        }
        else if(MCU_HDR_FF == data)
        {
            isSyncHead = 1;
        }
        offRec++;
    }

    // 从串口缓冲区中找到最近一帧数据，返回给应用层
    ret = find_packet_forward(pos_start, &packetLen);
    if(ret >= 0)
    {
        /* a whole packet */
        pos_start = ret;
        for(i = 0; i < packetLen; i++)
        {
            buf[i] = __halbuf_read(pos_start + i);
        }
        pos_start += packetLen;
        
        return packetLen;
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