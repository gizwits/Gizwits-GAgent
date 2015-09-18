#include "gagent.h"
#include "hal_receive.h"

uint8 *hal_RxBuffer=NULL;
/*
 pos_start:pos of get
 pos_current:pos of put
 */
static uint32 pos_start = 0;
static uint32 pos_current = 0;

/*
 * 串口通讯协议
 * | head(0xffff) | len(2B) | cmd(2B) | SN(1B) | flag(2B) | payload(xB) | checksum(1B) |
 *     0xffff     cmd~checksum                                            len~payload
 *     0xff-->LOCAL_HAL_REC_SYNCHEAD1
 *         ff->LOCAL_HAL_REC_SYNCHEAD2
 *                  len_1-->LOCAL_HAL_REC_DATALEN1
 *                  len_2-->LOCAL_HAL_REC_DATALEN2
 *                          | ------------     LOCAL_HAL_REC_DATA     -----------------|
 *                | ------- halRecKeyWord set to 1 while rec first byte 0xff ----------|
 */
/****************LOCAL MACRO AND VAR**********************************/
// for gu8LocalHalStatus
    #define         LOCAL_HAL_REC_SYNCHEAD1     1
    #define         LOCAL_HAL_REC_SYNCHEAD2     2
    #define         LOCAL_HAL_REC_DATALEN1      3
    #define         LOCAL_HAL_REC_DATALEN2      4
    #define         LOCAL_HAL_REC_DATA          5
static uint8 gu8LocalHalStatus = LOCAL_HAL_REC_SYNCHEAD1;
/* 见上协议，len字段 */
static uint16 gu16LocalPacketDataLen;
/* 一包数据的气味位置偏移 */
static uint32 guiLocalPacketStart;
/* 非包头部分，数据0xff应该变换为0xff 55,不影响校验和和len。因此，非包头部分，收到0xff后，该标志位被置1 */
static uint8 halRecKeyWord = 0;

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
FunctionName    :   GAgent_Local_ExtractOnePacket
Description     :   extract one packet from local cycle buf, and 
                    put data into buf.Will change pos_start
buf             :   dest buf
return          :   >0 the local packet data length.
                    <0 don't have one whole packet data
****************************************************************/
int32 GAgent_Local_ExtractOnePacket(uint8 *buf)
{
    uint8 data;
    uint32 i = 0;

    while((pos_start & HAL_BUF_MASK) != (pos_current & HAL_BUF_MASK))
    {
        data = __halbuf_read(pos_start);

        if(LOCAL_HAL_REC_SYNCHEAD1 == gu8LocalHalStatus)
        {
            if(MCU_HDR_FF == data)
            {
                gu8LocalHalStatus = LOCAL_HAL_REC_SYNCHEAD2;

                guiLocalPacketStart = pos_start;
            }
        }
        else if(LOCAL_HAL_REC_SYNCHEAD2 == gu8LocalHalStatus)
        {
            if(MCU_HDR_FF == data)
            {
                gu8LocalHalStatus = LOCAL_HAL_REC_DATALEN1;
            }
            else
            {
                gu8LocalHalStatus = LOCAL_HAL_REC_SYNCHEAD1;
            }
        }
        else
        {
            if(halRecKeyWord)
            {
                /* 前面接收到0xff */
                halRecKeyWord = 0;
                if(0x55 == data)
                {
                    data = 0xff;
                    move_data_backward(hal_RxBuffer, pos_start + 1, pos_current, 1);
                    pos_current--;
                    pos_start--;

                }
                else if(MCU_HDR_FF == data)
                {
                    /* 新的一包数据，前面数据丢弃 */
                    gu8LocalHalStatus = LOCAL_HAL_REC_DATALEN1;
                    guiLocalPacketStart = pos_start - 1;
                    pos_start++;
                    continue;
                }
                else
                {
                    if(LOCAL_HAL_REC_DATALEN1 == gu8LocalHalStatus)
                    {
                        /* 说明前面接收到的0xff和包头0xffff是连在一起的，以最近的0xffff作为包头起始，
                         * 当前字节作为len字节进行解析
                         */
                        guiLocalPacketStart = pos_start - 2;
                    }
                    else
                    {
                        gu8LocalHalStatus = LOCAL_HAL_REC_SYNCHEAD1;
                        pos_start++;
                        continue;
                    }
                }

            }
            else
            {
                if(MCU_HDR_FF == data)
                {
                    halRecKeyWord = 1;
                    pos_start++;
                    continue;
                }
            }

            if(LOCAL_HAL_REC_DATALEN1 == gu8LocalHalStatus)
            {
                gu16LocalPacketDataLen = data;
                gu16LocalPacketDataLen = (gu16LocalPacketDataLen << 8) & 0xff00;
                gu8LocalHalStatus = LOCAL_HAL_REC_DATALEN2;
            }
            else if(LOCAL_HAL_REC_DATALEN2 == gu8LocalHalStatus)
            {
                gu16LocalPacketDataLen += data;
                gu8LocalHalStatus = LOCAL_HAL_REC_DATA;

                if(0 == gu16LocalPacketDataLen)
                {
                    /* invalid packet */
                    gu8LocalHalStatus = LOCAL_HAL_REC_SYNCHEAD1;
                }
            }
            else
            {
                /* Rec data */
                gu16LocalPacketDataLen--;
                if(0 == gu16LocalPacketDataLen)
                {
                    /* 接收到完整一包数据，拷贝到应用层缓冲区 */
                    pos_start++;
                    i = 0;
                    while(guiLocalPacketStart != pos_start)
                    {
                        buf[i] = __halbuf_read(guiLocalPacketStart++);
                        i++;
                    }

                    return i;
                }
            }
            
        }
        
        pos_start++;
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

    if( pgc->rtinfo.local.uart_fd >= 0 )
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
        if(read_count <= 0)
        {
            return read_count;
        }

        pos_current += read_count;

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