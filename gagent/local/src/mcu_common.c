#include "gagent.h"
/*****************************************************************
*   fd      :   fd>0 is a socketid
*   buf     :   wifi 2 mcu buf.
*   bufLen  :   wifi 2 mcu bufLen
*   pRxbuf  :   data from local.
*   Time    :   current time ms
*   return  :   0 check success   1 check fail
*   Add by alex lin 2014-09-02
*******************************************************************/
int GAgent_CheckAck( int fd, pgcontext pgc,unsigned char *buf,int bufLen,ppacket pRxbuf,u32 time )
{
    int i=0;
    int PacketLen=0;
    int resend_time=1;
    unsigned char checksum=0;
    uint8 *phead =NULL;
    int sum=0;
    int8 cmd =0;
    while(1)
    {
        GAgent_SelectFd( pgc,0,1000 );
        PacketLen = GAgent_Local_GetPacket( pgc,pRxbuf );
        if( PacketLen>0 )
        {
            phead = pRxbuf->phead;
            checksum = GAgent_SetCheckSum( phead,(PacketLen-1));        
            /*(PacketLen-1) remove the last byte, checksum*/
            if( (phead[4]==(buf[4]+1))&& /*need careful about protocol*/
                (buf[5]==phead[5]))
            {
                if ((checksum==phead[PacketLen-1]))
                {
                    cmd = phead[4];
                    //send data to app.
                    if(  PacketLen>9 )
                    {
                        if( MCU_CTRL_CMD_ACK ==cmd )
                        {
                            pRxbuf->type = SetPacketType( pRxbuf->type,LOCAL_DATA_IN,1 );
                            GAgent_Printf( GAGENT_INFO,"Send ACK to APP ");
                            ParsePacket( pRxbuf );
                            dealPacket( pgc, pRxbuf );
                        }
                    }          
                    GAgent_Printf(GAGENT_INFO,"Check MCU ACK OK!");
                    return 0;
                }
                else
                {
                    // TODO 
                    GAgent_Printf(GAGENT_INFO, "Get ACK failed, checksum info: %d:%d %d", 
                            checksum, phead[PacketLen-1], PacketLen);
                
                    GAgent_Printf(GAGENT_INFO, "Send Packet Info:");
                    GAgent_DebugPacket(buf, bufLen);

                    GAgent_Printf(GAGENT_INFO, "Received Packet Info:");
                    GAgent_DebugPacket(phead, PacketLen);
                }
                return 1;
            }
            else
            {
                //TODO 
            }
        }
        else if( abs(GAgent_GetDevTime_MS()-time) >= MCU_ACK_TIME_MS*1 )
        {
            GAgent_Printf(GAGENT_INFO,"Time ms %d",GAgent_GetDevTime_MS() );
            time=GAgent_GetDevTime_MS();
            if( resend_time>=3 )
            {
                GAgent_Printf(GAGENT_INFO, "Get ACK failed at %s:%d, resend_time:%d packet info:", __FUNCTION__, __LINE__,resend_time);
                GAgent_DebugPacket(buf, bufLen);
                return 1;
            }
            GAgent_Printf( GAGENT_INFO,"resend_time=%d",resend_time );
            Local_SendData( fd,buf,bufLen );
            resend_time += 1;            
        }
        //msleep(10);
    }
}