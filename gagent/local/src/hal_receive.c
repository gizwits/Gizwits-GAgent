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
            datalen = get_data_len(offset, pos + 1);
            packetlen = __halbuf_read(offset + MCU_LEN_POS);
            packetlen = (packetlen << 8) | __halbuf_read(offset + MCU_LEN_POS + 1);
            packetlen += MCU_SYNC_HEAD_LEN + sizeof(packetlen);
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
    uint16 packetLenMin;
    uint32 start, end;

    start = pos;
    end = pos_current;
    datalen = get_data_len(start, end);

    /* A packet len must larger than the member sync head + len */
    packetLenMin = MCU_SYNC_HEAD_LEN + sizeof(packetlen);
    
    while( datalen > packetLenMin )
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
        datalen--;
    }

    return RET_FAILED;
}

int32 check_isInPacket(uint32 pos)
{
    uint32 packetStart;
    uint8 data1, data2;
    uint32 datalen;
    uint32 packetLen;

    packetStart = find_packet_backward(pos);
    
    data1 = __halbuf_read(packetStart);
    data2 = __halbuf_read(packetStart + 1);

    if(MCU_HDR_FF == data1 && MCU_HDR_FF == data2)
    {
        datalen = get_data_len(packetStart, pos) + 1;
        
        if(datalen <= MCU_SYNC_HEAD_LEN + sizeof(uint16))
        {
            return 0;
        }
        
        packetLen = __halbuf_read(packetStart + MCU_LEN_POS);
        packetLen = (packetLen << 8) | __halbuf_read(packetStart + MCU_LEN_POS + 1);
        packetLen += MCU_SYNC_HEAD_LEN + sizeof(uint16);

        if(datalen > packetLen)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

/****************************************************************
FunctionName    :   Local_decode_data
Description     :   decode local data in local cycle buf.transfer 0xff55 to 0xff.
start           :   the posation in local cycle buf where will be decode  
length          :   decode data length

will change the global pointer "put"--pos_current
****************************************************************/
void hal_decode_data(uint32 start, uint32 length)
{
    static uint8 isSyncHead = 0;
    uint8 data;
    uint32 i;
    uint32 offRec;
    uint32 invalidLen;
    int32 ret;
    
    if(0 == length)
    {
        return ;
    }

    offRec = start;
    for(i = 0; i < length; i++)
    {
        data = __halbuf_read(offRec);
        if(isSyncHead)
        {
            isSyncHead = 0;
            /* check if is a new packet */
            if(0xFF == data)
            {
                /* check if pre byte is in one packet */
                ret = check_isInPacket(offRec - 1);
                if(0 == ret)
                {   /* new packet */
                    offRec++;
                    continue;
                }
                else
                {
                    ret = find_packet_backward(offRec);
                    invalidLen = get_data_len(ret, offRec + 1);
                    move_data_backward(hal_RxBuffer, offRec + 1, pos_current, invalidLen);
                    offRec -= invalidLen;
                    pos_current = pos_current - invalidLen;
                }
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
                move_data_backward(hal_RxBuffer, offRec + 1, pos_current, invalidLen);
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
}

/****************************************************************
FunctionName    :   GAgent_Local_ExtractOnePacket
Description     :   extract one packet from local cycle buf, and 
                    put data into buf.Will change pos_start
buf             :   dest buf
return          :   >0 the local packet data length.
                    <0 don't have one whole packet data
****************************************************************/
int32 GAgent_Local_ExtractOnePacket(uint8 *buf)
{
    int32 ret;
    uint32 i;
    uint16 packetLen;

    if(NULL == buf)
    {
        GAgent_Printf(GAGENT_WARNING,"%s,%d,Input buf is NULL", __FUNCTION__, __LINE__);
        return RET_FAILED;
    }

    /* find one packet data form position pos_start of local driver buf */
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

    return RET_FAILED;
}

/****************************************************************
FunctionName    :   GAgent_Local_WaitDataReady.
Description     :   wait until local serial bus has data reached or timeout
pgc             :   global context
timeoutS        :   Second.totla timeout = timeoutS + timeoutMs
timeoutMs       :   mill second.totla timeout = timeoutS + timeoutMs
****************************************************************/
int32 GAgent_Local_WaitDataReady(pgcontext pgc, uint32 timeoutS, uint32 timeoutMs)
{
    int32 maxFd;
    int32 ret=0;

    FD_ZERO( &(pgc->rtinfo.readfd) );

    if( pgc->rtinfo.local.uart_fd>0 )
    {
        FD_SET( pgc->rtinfo.local.uart_fd,&(pgc->rtinfo.readfd));
        maxFd = pgc->rtinfo.local.uart_fd + 1;
        ret = GAgent_select( maxFd, &(pgc->rtinfo.readfd), NULL, NULL,
           timeoutS,timeoutMs*1000 );
    }

    return ret;
}

/****************************************************************
FunctionName    :   GAgent_Local_IsDataValid.
Description     :   Check if is data valid realley.
pgc             :   global context
****************************************************************/
int32 GAgent_Local_IsDataValid(pgcontext pgc)
{
    if(FD_ISSET( pgc->rtinfo.local.uart_fd, &(pgc->rtinfo.readfd)))
    {
        return RET_SUCCESS;
    }
    else
    {
        return RET_FAILED;
    }
}

/****************************************************************
FunctionName    :   GAgent_Local_RecAll
Description     :   receive all of data form local io one time.
                    and put data int local cycle buf.This func will
                    change data 0xff55 isn't SYNC HEAD to 0xff
pgc             :   gagent global struct. 
return          :   void.
****************************************************************/
int32 GAgent_Local_RecAll(pgcontext pgc)
{
    int32 fd;
    int32 available_len =0;
    int32 read_count = 0;
    uint32 offRec;

    fd = pgc->rtinfo.local.uart_fd;
    if(fd < 0)
    {
        return RET_FAILED;
    }

    if(RET_SUCCESS == GAgent_Local_IsDataValid(pgc))
    {
        /* step 1.read data into loopbuf */
        available_len = get_available_buf_space( pos_current, pos_start );
        read_count = serial_read( fd, &hal_RxBuffer[pos_current & HAL_BUF_MASK],available_len );
        offRec = pos_current;
        if(read_count <= 0)
        {
            return read_count;
        }

        pos_current += read_count;

        /* step 2. transfer 0xff55 to 0xff */
        hal_decode_data(offRec, read_count);

        read_count = get_data_len(offRec, pos_current);
    }

    return read_count;
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