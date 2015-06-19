#include "gagent.h"
#include "cloud.h"
#include "3rdlan.h"
#include "3rdcloud.h"

/*****************************************************************************
JD UDP port is  :   80 or 4320
udp_port        :   udp port (80/4320)
return          :   socket id.
add by alex lin --2014-11-06
******************************************************************************/
int32 Socket_CreateUDPServer_JD(int32 udp_port)
{
    return GAgent_CreateUDPServer( udp_port );
}
/*****************************************************************************
FUNCTION    :   build JD udp header.
len         :   data len except the common_header len.
checksum    :   data checksum
type        :   data type 
buf         :   header include common_header and cmd_header (19byte)
return      :    NULL
add by alex lin --2014-11-06.
*****************************************************************************/
void GAgent_JD_Build_ACK(unsigned int type,char *buf,int bufLen )
{
    int len=0;
    //aa 55 00 00
    buf[0] = 0xaa;
    buf[1] = 0x55;
    buf[2] = 0x00;
    buf[3] = 0x00;
    //len 
    len=bufLen-13;//6 the cmd_header len.
    buf[4] = len&0xff;
    buf[5] = (len>>8)&0xff;
    buf[6] = (len>>16)&0xff;
    buf[7] = (len>>24)&0xff;
    //enctype alway 0
    buf[8] = 0x00;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x00;

    //CMD_Header(6b)
    //type
    buf[13] = type&0xff;
    buf[14] = (type>>8)&0xff;
    buf[15] = (type>>16)&0xff;
    buf[16] = (type>>24)&0xff;
    //cmd 2b :no useful
    buf[17] = 0x4f;
    buf[18] = 0x4b;
    //checksum
    buf[12] = GAgent_CheckSum(buf+13,bufLen-13);
}
void GAgent_JD_Discover_Ack( int32 udp_socket,struct sockaddr_t *addr,uint8 *szMac,int8 *uuid,int8 *feed_id )
{
    int32 i=0;
    int8 *buf_ack=NULL;
    int32 totalLen=0,dataLen=0,pos=0;
    int32 macLen=0;
    int32 productuuidLen=0;
    int32 feedidLen=0;
    int8 strMacByte[3] = {0};
    int8 Mac[6]={0};
    int8 mac[50]={0};
    int8 productuuid[50]={0};
    //char *productuuid = "\"productuuid\":\"A4ZY7J\",";
    int8 Temp_feed_id[100]={0};
    int8 *feedid = "\"feedid\":\"\"}";

        //mac    
        strMacByte[2] = 0;
    for(i=0;i<6;i++)
    {
        strMacByte[0] = szMac[i*2];
        strMacByte[1] = szMac[i*2 + 1];
        Mac[i] = strtoul(strMacByte, NULL, 16);
    }
    if(strlen((const int8* )uuid)<=0) 
    {
        GAgent_Printf( GAGENT_INFO,"no uuid...\r\n ");
        return ;
    }
    GAgent_Printf( GAGENT_INFO,"have uuid: %s\r\n",uuid);
    sprintf( productuuid,"\"productuuid\":\"%s\",",( const int8*)uuid);
    if(strlen(feed_id)<=0)
    {
        strcpy(Temp_feed_id,feedid);
        GAgent_Printf( GAGENT_INFO,"no feed_id...\r\n ");
    }
    else
    {
        sprintf( Temp_feed_id,"\"feedid\":\"%s\"}",feed_id);
        GAgent_Printf( GAGENT_INFO,"have feed_id:%s \r\n",feed_id );
    }
    sprintf(mac,"{\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",",Mac[0],Mac[1],Mac[2],Mac[3],Mac[4],Mac[5]);
    pos+=JD2HEADER_LEN;
    macLen = strlen(mac);
    productuuidLen = strlen(productuuid);
    feedidLen = strlen(Temp_feed_id);

    dataLen = macLen+productuuidLen+feedidLen;
    /*19byte is two header byte(common header and cmd header)*/
    totalLen = dataLen+JD2HEADER_LEN;

    buf_ack = (int8*)malloc(totalLen);
    if(buf_ack==NULL) return ;
        
    memcpy(buf_ack+pos,mac,macLen);
    pos+=macLen;
    memcpy(buf_ack+pos,productuuid,productuuidLen);
    pos+=productuuidLen;
    memcpy( buf_ack+pos,Temp_feed_id,feedidLen);
    pos+=feedidLen;

    GAgent_JD_Build_ACK( APP2WIFI_BROADCAST_ACK,buf_ack,totalLen);

    i = Socket_sendto(udp_socket, (uint8*)buf_ack, totalLen, addr, sizeof(struct sockaddr_t) );
    //GAgent_DebugPacket( (uint8*)buf_ack, totalLen);
    GAgent_Printf(GAGENT_INFO,"----ACK Discover-sendto return:%d, but need:%d,----", i, totalLen);
    free( buf_ack );
}
/*****************************************************************************
Function        :   GAgent_JD_Get_Feedid_Key
Description     :   get the accesskey and feedid from jd udp data.
gagent3rdcloud  :   JD info struct ponter.
buf             :   JD udp data.
return          :   -1:fail
                    0:ok and need to updata new jdinfo
                    1:ok no new jdinfo
Add by Alex.lin     --2015-06-11
*****************************************************************************/
int32 GAgent_JD_Get_Feedid_Key( GAgent3Cloud  *gagent3rdcloud,int8 *buf )
{
    int8 *p_start=NULL;
    int8 *p_end=NULL;
    int8 isChange=0;
    p_start = strstr( buf+19,"\"accesskey\"");
    if(p_start==NULL)
    {
        GAgent_Printf(GAGENT_INFO,"can't find the accesskey ");
        return RET_FAILED;
    }
    p_start +=(strlen("\"accesskey\"")+2);

    p_end = p_start;
    p_end = strstr( p_start,"\"");
    if(p_end==NULL)
    {
        return RET_FAILED;
    }
    if( 0!=memcmp( gagent3rdcloud->jdinfo.access_key,p_start,(p_end-p_start) ) )
    {
        isChange=1;
        memcpy( gagent3rdcloud->jdinfo.access_key,p_start,(p_end-p_start) );
        gagent3rdcloud->jdinfo.access_key[p_end-p_start]='\0';
        GAgent_Printf( GAGENT_DEBUG,"Got a new accesskey:%s",gagent3rdcloud->jdinfo.access_key );
    }

    p_start = strstr(p_end,"\"feedid\"");
    if(p_start==NULL)
    {
        return RET_FAILED;
    }
    p_start+=(strlen("\"feedid\"")+2);
    p_end =strstr(p_start,"\"");
    if(p_end==NULL)
    {
        return RET_FAILED;
    }
    if( 0!=memcmp(gagent3rdcloud->jdinfo.feed_id,p_start,(p_end-p_start)) )
    {
        isChange=1;
        memcpy(gagent3rdcloud->jdinfo.feed_id,p_start,(p_end-p_start));
        gagent3rdcloud->jdinfo.feed_id[p_end-p_start]='\0';
        GAgent_Printf( GAGENT_DEBUG,"Got a new feedid:%s",gagent3rdcloud->jdinfo.feed_id );
    }
    if( 0==isChange )
        return 1;
    else
    {
        gagent3rdcloud->jdinfo.tobeuploaded = 1;
        gagent3rdcloud->jdinfo.ischanged = 1;
        return 0;
    }
}
void GAgent_JD_Write_Ack( int32 udpsocket,struct sockaddr_t *addr,int32 flag )
{
    int32 ret;
    int8 *data_ok="{\"code\":0,\"msg\":\"network init success\"}";
    int8 *data_no="{\"code\":2,\"msg\":\"network init fail\"}";
    int32 dataLen = 0,totalLen=0,pos=0;
    int8 *buf_ack=NULL;
    if(flag<0)
    {
        dataLen = strlen(data_no);
    }
    else
    {
        dataLen = strlen(data_ok);
    }
    
    totalLen = dataLen+JD2HEADER_LEN;
    buf_ack = (int8*)malloc(totalLen);
    if( buf_ack==NULL )   return ;
    pos+=JD2HEADER_LEN;
    memcpy(buf_ack+pos,data_ok,dataLen );
    if(flag>=0)
    {
        memcpy(buf_ack+pos,data_ok,dataLen );
        GAgent_Printf(GAGENT_DEBUG,"----GAgent_JD_Write_Ack success---");
    }
    else
    {
        memcpy(buf_ack+pos,data_no,dataLen );
        GAgent_Printf(GAGENT_DEBUG,"----GAgent_JD_Write_Ack fail---");
    }
    GAgent_JD_Build_ACK( APP2WIFI_WRITE_ACK,buf_ack,totalLen );
    ret = Socket_sendto(udpsocket, (uint8*)buf_ack, totalLen, addr, sizeof(struct sockaddr_t));
    GAgent_Printf(GAGENT_DEBUG, "JD Sentto Num:%d \r\n", ret);
    free(buf_ack);
    return ;
}
void Lan3rdCloudUDPHandle_JD( pgcontext pgc,struct sockaddr_t *paddr,
            ppacket prxBuf, int32 recLen )
{
    uint8 *JDSocketbuffer=NULL;
    uint8 checksum=0;
    uint32 udpbodyLen=0;
    uint32 enctype=0;
    uint32 i=0,ret=0;
    
    JDSocketbuffer = prxBuf->phead;
    if( !((JDSocketbuffer[0]==0xaa)&&(JDSocketbuffer[1]==0x55)))
    {
        ERRORCODE
        GAgent_Printf( GAGENT_DEBUG,"JD lan udp Receive error data.");
        return;
    }
    udpbodyLen = JDSocketbuffer[4]+JDSocketbuffer[5]*256+JDSocketbuffer[6]*256*256+JDSocketbuffer[7]*256*256*256;
    if( udpbodyLen!=(recLen-13))
        {
            GAgent_Printf( GAGENT_DEBUG,"JD UDP bodylen =%d right len=%d",udpbodyLen,recLen-13 );
            return;
            //continue;
        }
    enctype = JDSocketbuffer[8]+JDSocketbuffer[9]*256+JDSocketbuffer[10]*256*256+JDSocketbuffer[11]*256*256*256;	
    if(enctype!=0)
        {
            GAgent_Printf( GAGENT_DEBUG,"JD UDP Data type error : %d ",enctype );
            return;
            // JD no use aes don't need to do.
            //continue ;
        }
    for( i=0;i<udpbodyLen;i++ )
      {
          checksum = checksum+JDSocketbuffer[13+i];
      }
    if( checksum!=JDSocketbuffer[12] )
    {
      GAgent_Printf( GAGENT_DEBUG,"JD UDP Data checksum error, GAgent checksum:%d  JD app checksum:%d\r\n",checksum,JDSocketbuffer[12] );
      return ;
    }
    enctype = JDSocketbuffer[13]+JDSocketbuffer[14]*256+JDSocketbuffer[15]*256*256+JDSocketbuffer[16]*256*256*256;
    switch( enctype )
        {
        case APP2WIFI_BROADCAST:
            GAgent_Printf( GAGENT_INFO,"-------- JD UDP Discover --------");
            GAgent_JD_Discover_Ack( pgc->ls.udp3rdCloudFd,paddr,pgc->minfo.szmac,
                                    pgc->gc.cloud3info.jdinfo.product_uuid,
                                    pgc->gc.cloud3info.jdinfo.feed_id);
            break;
        case APP2WIFI_WRITE:
            GAgent_Printf( GAGENT_INFO,"-------- JD UDP Write data to GAgent --------");
            ret = GAgent_JD_Get_Feedid_Key( &(pgc->gc.cloud3info),(char *)JDSocketbuffer );
            if( 0==ret )
            {
                GAgent_DevSaveConfigData( &(pgc->gc) );
                GAgent_Printf( GAGENT_INFO,"new accesskey:%s len=%d",
                                pgc->gc.cloud3info.jdinfo.access_key,
                                strlen(pgc->gc.cloud3info.jdinfo.access_key));
                GAgent_Printf( GAGENT_INFO,"new feedid:%s len=%d",
                                pgc->gc.cloud3info.jdinfo.feed_id,
                                strlen(pgc->gc.cloud3info.jdinfo.feed_id));
            }
            else if( 1==ret )
            {
                GAgent_Printf( GAGENT_INFO,"accesskey:%s len=%d",
                                pgc->gc.cloud3info.jdinfo.access_key,
                                strlen(pgc->gc.cloud3info.jdinfo.access_key));
                GAgent_Printf( GAGENT_INFO,"feedid:%s len=%d",
                                pgc->gc.cloud3info.jdinfo.feed_id,
                                strlen(pgc->gc.cloud3info.jdinfo.feed_id));
            }
            GAgent_JD_Write_Ack( pgc->ls.udp3rdCloudFd,paddr,ret );
            if( 1==pgc->gc.cloud3info.jdinfo.tobeuploaded )
            {
                GAgent_SetCloudConfigStatus ( pgc,CLOUD_RES_POST_JD_INFO );
                ret = Cloud_JD_Post_ReqFeed_Key( pgc );
            }
            break;
        default:
            GAgent_Printf( GAGENT_INFO,"JD enctype invalid.");
            break;
        }
        GAgent_Printf( GAGENT_INFO,"\r\n" );
}

void GAgent3rdLan_Handle( pgcontext pgc )
{
    if( 1!= pgc->rtinfo.waninfo.Cloud3Flag )
        return ;
    if( 0==strcmp( pgc->gc.cloud3info.cloud3Name,"jd") )
    {   /* 处理京东小循环事务 */
        ppacket  prxBuf=NULL;
        int32 recLen=0;
        struct sockaddr_t addr;
        int addrLen = sizeof(struct sockaddr_t);
        
        prxBuf = pgc->rtinfo.Rxbuf;
        if( pgc->ls.udp3rdCloudFd>0 )
        {
            if(FD_ISSET(pgc->ls.udp3rdCloudFd, &(pgc->rtinfo.readfd)))
            {
                resetPacket(prxBuf);
                recLen = Socket_recvfrom(pgc->ls.udp3rdCloudFd, prxBuf->phead, GAGENT_BUF_LEN,
                    &addr, (socklen_t *)&addrLen);
                GAgent_Printf( GAGENT_INFO,"3rdCloud Lan Receive len = %d error=%d ",recLen,errno );
                if( recLen<=0 ) return ;
                GAgent_Printf(GAGENT_INFO,"Do JD udp Data handle.");
                Lan3rdCloudUDPHandle_JD( pgc,&addr, prxBuf , recLen );
            }
        }
    }
}