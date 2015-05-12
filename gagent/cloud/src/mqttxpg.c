#include "gagent.h"
#include "mqttxpg.h"
#include "mqttbase.h"
#include "mqttxpg.h"
#include "mqttlib.h"
#include "cloud.h"


static mqtt_broker_handle_t g_stMQTTBroker;

int32 MQTT_readPacket( int32 socketid,ppacket pbuf,int32 bufferLen )
{
    int32 bytes_rcvd;
    uint8_t *pData;
    int32 messageLen;
    int32 varLen;
    int32 packet_length;

    memset(pbuf->phead, 0, bufferLen);
    pData = pbuf->phead;
    bytes_rcvd = recv(socketid, pData , bufferLen, 0);
    if((bytes_rcvd) <= 0)
    {
        MQTTclose_socket( &g_stMQTTBroker );
        return -1;
    }
    //pData = packetBuffer + 0;
    pbuf->pend=pbuf->phead+bytes_rcvd;
    
    messageLen = mqtt_parse_rem_len(pData);
    varLen = mqtt_num_rem_len_bytes(pData);
    packet_length = varLen + messageLen + 1;

    if (bytes_rcvd < packet_length)
    {
        GAgent_Printf(GAGENT_INFO, " packet length too long %s:%d ", __FUNCTION__, __LINE__);
        return -3;
    }
   
    return bytes_rcvd;
}
int32 MQTTclose_socket(mqtt_broker_handle_t* broker)
{
    int32 fd = broker->socketid;
    close(fd);
    broker->socketid = -1;
    return 0;
}

/**************************************************
 *
 *
 *      return :0
 *
 ***************************************************/
static int32 init_subTopic( mqtt_broker_handle_t* broker ,pgcontext pgc,char *Sub_TopicBuf,int32 Mqtt_Subflag)
{
    int32 productKeyLen=0,DIdLen=0;
    
    DIdLen = strlen( pgc->gc.DID );
    switch(Mqtt_Subflag)
    {
    case 1:
        //4.6
        productKeyLen = strlen( pgc->mcu.product_key );
        memcpy( Sub_TopicBuf,"ser2cli_noti/",strlen("ser2cli_noti/") );
        memcpy( Sub_TopicBuf+strlen("ser2cli_noti/"),pgc->mcu.product_key,productKeyLen );
        Sub_TopicBuf[strlen("ser2cli_noti/")+productKeyLen] = '\0';
        break;
    case 2:
        // 4.7 4.9
        memcpy( Sub_TopicBuf,"ser2cli_res/",strlen("ser2cli_res/") );
        memcpy( Sub_TopicBuf+strlen("ser2cli_res/"),pgc->gc.DID,DIdLen );
        Sub_TopicBuf[strlen("ser2cli_res/")+DIdLen] = '\0';
        break;
    case 3:
        // 4.13
        memcpy(Sub_TopicBuf,"app2dev/",strlen("app2dev/") );
        memcpy(Sub_TopicBuf+strlen("app2dev/"),pgc->gc.DID,DIdLen );
        Sub_TopicBuf[strlen("app2dev/")+DIdLen] = '/';
        Sub_TopicBuf[strlen("app2dev/")+DIdLen+1] = '#';
        Sub_TopicBuf[strlen("app2dev/")+DIdLen+2] = '\0';
        break;
    default:
        break;
    }

    return 0;
}
int32 Mqtt_DoSubTopic( pgcontext pgc,int16 mqttstatus )
{
    int8 ret =0;
    ret = Mqtt_SubLoginTopic( &g_stMQTTBroker,pgc,mqttstatus );
    return ret;
}
/*******************************************************
 *
 * 返回成功订阅Topic的个数
 *
 ********************************************************/
int32 Mqtt_SubLoginTopic( mqtt_broker_handle_t *LoginBroker,pgcontext pgc,int16 mqttstatus )
{
    char Sub_TopicBuf[100];
    char Topic[100];

    memset(Sub_TopicBuf,0,100);
    memset(Topic,0,100);
    
    switch(mqttstatus)
    {
    case MQTT_STATUS_REQ_LOGINTOPIC1:
        init_subTopic(LoginBroker,pgc,Sub_TopicBuf,1);
        if(mqtt_subscribe( LoginBroker,Sub_TopicBuf,&(pgc->rtinfo.waninfo.mqttMsgsubid) )==1)
        {   
            sprintf(Topic,"LOGIN sub topic1 is:%s",Sub_TopicBuf);
            GAgent_Printf(GAGENT_INFO,Topic);
            GAgent_Printf(GAGENT_INFO,"MQTT_STATUS_LOGINTOPIC1");
        }
        break;            
    case MQTT_STATUS_REQ_LOGINTOPIC2:
        init_subTopic(LoginBroker,pgc,Sub_TopicBuf,2);
        if(mqtt_subscribe( LoginBroker,Sub_TopicBuf,&(pgc->rtinfo.waninfo.mqttMsgsubid) )==1)
        {    
            sprintf(Topic,"LOGIN T2 sub topic is:%s",Sub_TopicBuf);                                
            GAgent_Printf(GAGENT_INFO,Topic);
            GAgent_Printf(GAGENT_INFO,"MQTT_STATUS_LOGINTOPIC2");
        }
        break;
    case MQTT_STATUS_REQ_LOGINTOPIC3:
        init_subTopic(LoginBroker,pgc,Sub_TopicBuf,3);
        if(mqtt_subscribe( LoginBroker,Sub_TopicBuf,&(pgc->rtinfo.waninfo.mqttMsgsubid) )==1)
        {
            sprintf(Topic,"LOGIN T3 sub topic is:%s",Sub_TopicBuf);
            GAgent_Printf(GAGENT_INFO,Topic);
            GAgent_Printf(GAGENT_INFO,"MQTT_STATUS_LOGINTOPIC3");
        }
        break;
    default:
        break;
    }

    return 0;
}

/***********************************************************
 *
 *   return :    0 success ,1 error
 *
 *************************************************************/
int32 Mqtt_SendConnetPacket( mqtt_broker_handle_t *pstBroketHandle, int32 socketid,const int8* username,const int8* password )
{       
    int32 ret;
    
    if( (username==NULL) || (password==NULL) )// 匿名登录 GAgent V4 will not runing this code.
    {
        return 1;
    }
    else // 用户登录
    {
        mqtt_init( pstBroketHandle,username );
        mqtt_init_auth(pstBroketHandle,username,password );
    }
    mqtt_set_alive(pstBroketHandle, CLOUD_MQTT_SET_ALIVE);
    pstBroketHandle->socketid = socketid;
    pstBroketHandle->mqttsend = send_packet;

    ret = mqtt_connect(pstBroketHandle);
    if (ret != 0)
        {
            GAgent_Printf(GAGENT_WARNING,"mqtt send connect packet is failed with:%d", ret); 
            return 1;
        }
    GAgent_Printf(GAGENT_INFO, "Mqtt_SendConnetPacket OK, write:%d", ret); 
    return 0;
}


int32 Mqtt_Login2Server(  int32 socketid,const uint8 *username,const uint8 *password )
{
    if( Mqtt_SendConnetPacket( &g_stMQTTBroker,socketid,username,password ) == 0)
    {    
        GAgent_Printf(GAGENT_INFO," Mqtt_SendConnetPacket OK!");
        return 0;
    }   
    GAgent_Printf(GAGENT_INFO," Mqtt_SendConnetPacket NO!");    
    
    return 1;
}

void MQTT_HeartbeatTime( void )
{
    mqtt_ping(&g_stMQTTBroker);
}

int32 Mqtt_IntoRunning( pgcontext pgc )
{
    mqtt_ping(&g_stMQTTBroker);
    Mqtt_ReqOnlineClient();
    return 0;
}
// 发送日志
// 目前使用全局变量交换数据
// pmbroker:MQTT Broker, lm:Log Manager
void SendLog( /*mqtt_broker_handle_t *pmbroker, LogMan *lm*/ )
{
    
    return;
}

void Log2Cloud( pgcontext pgc )
{
    //SendLog(&g_stMQTTBroker, pgc->logman/*g_Xpg_GlobalVar.logman*/);
    return;
}
/********************************************************************
        Function        :   MQTT_SenData.
        Description     :   send buf data to m2m.
        szDID           :   Device ID.
        buf             :   need to send data pointer.
        buflen          :   buf length .
        return          :   0-ok 
                            other fail.
        Add by Alex.lin     --2015-03-17
********************************************************************/
int32 MQTT_SenData( int8 *szDID, ppacket pbuf,/*uint8 *buf,*/ int32 buflen )
{
    uint8 *sendpack=NULL;
    int32 i=0,sendpacklen=0,headlen=0;
    int16 varlen=0;
    volatile varc sendvarc;
    int8 msgtopic[40]= {0};
    int32 didlen=0;
    didlen = strlen(szDID);
    if(didlen!=22)
        return -1;
    
    memcpy( msgtopic,"dev2app/",strlen("dev2app/"));
    memcpy( msgtopic+strlen("dev2app/"),szDID,didlen );
    msgtopic[strlen("dev2app/")+didlen] = '\0';
    
    //protocolVer(4B)+varLen(1~4B)+flag(1B)+cmd(2B)+P0
    varlen = 1+2+buflen;
    sendvarc=Tran2varc(varlen);
    sendpacklen = 4+sendvarc.varcbty+1+2+buflen;
    headlen = sendpacklen-buflen;
    
    sendpack = ( (pbuf->ppayload)-headlen );
    //protocolVer
    sendpack[0] = 0x00;
    sendpack[1] = 0x00;
    sendpack[2] = 0x00;
    sendpack[3] = 0x03;
    //varLen
    for(i=0;i<sendvarc.varcbty;i++)
    {
        sendpack[4+i] = sendvarc.var[i];
    }
     //flag   
    sendpack[4+sendvarc.varcbty] = 0x00;
    //CMD
    sendpack[4+sendvarc.varcbty+1]=0x00;
    sendpack[4+sendvarc.varcbty+2]=0x91;
    

    GAgent_Printf(GAGENT_DEBUG,"------SEND TO Cloud ------\r\n");
    for(i=0;i<sendpacklen;i++)
        GAgent_Printf(GAGENT_DUMP," %02X",sendpack[i] );
    
    PubMsg( &g_stMQTTBroker,msgtopic,sendpack,sendpacklen,0 );

    return 0;
}
int32 MQTT_DoCloudMCUCmd(u8 clientid[32], u8 did[32], u8 *pHiP0Data, int32 P0DataLen)
{
    int32 varlen;
    int32 datalen;
    u8 *pP0Data;
    int32 pP0DataLen;
    int32 i;

    /* 根据报文中的报文长度确定报文是否是一个有效的报文 */
    varlen = mqtt_num_rem_len_bytes(pHiP0Data+4);
    /* 这个地方+3是因为MQTT库里面实现把 UDP flag算到messagelen里面，这里为了跟mqtt库保持一致所以加3 */
    datalen = mqtt_parse_rem_len(pHiP0Data+3); 
    
    pP0DataLen = datalen-3;// 因为 flag(1B)+cmd(2B)=3B
    
    // 到了payload开始的地方
    pP0Data = &pHiP0Data[7+varlen]; 

    //i = GAgentV4_Write2Mcu_with_p0( 0, MCU_CTRL_CMD,pP0Data,pP0DataLen );
    GAgent_Printf(GAGENT_INFO, "MCU Do CLOUD CMD return:%d", i);
    
    return 0;
}
/********************************************************************
 *
 *  FUNCTION   : Mqtt send request packbag to server ask online client
 *             add by alex.lin --2014-12-11
 ********************************************************************/
void Mqtt_ReqOnlineClient(void)
{
    char req_buf[6] = {0x00,0x00,0x00,0x03,0x02,0x0f};
    PubMsg( &g_stMQTTBroker,"cli2ser_req",req_buf,6,0);	
}
/********************************************************************
 *
 *  FUNCTION   : Mqtt send request packbag to server ask online client
 *               res.
 *   buf       : mqtt msg payload.
 *   return    : none.
 *             add by alex.lin --2014-12-11
 ********************************************************************/
void Mqtt_ResOnlineClient( pgcontext pgc,char *buf,int32 buflen)
{
    u16 *pWanclient_num;
    u16 wanclient_num=0;
    u16 lanclient_num=0;

    pWanclient_num = (u16*)&buf[6];
    wanclient_num = ntohs( *pWanclient_num );
    pgc->rtinfo.waninfo.wanclient_num = wanclient_num;
    lanclient_num = pgc->ls.tcpClientNums;

    if( 0 != wanclient_num)
    {
        GAgent_SetWiFiStatus( pgc,WIFI_CLIENT_ON,1 );
    }
    else if(0 == (wanclient_num + lanclient_num))
    {
        GAgent_SetWiFiStatus( pgc,WIFI_CLIENT_ON,0 );
    }

    GAgent_Printf(GAGENT_INFO,"wanclient_num = %d",wanclient_num );
    return ;
}
void Mqtt_Ack2Cloud( uint8 *pPhoneClient,uint8* szDID,uint8 *pData,uint32 datalen )
{
    uint8 msgtopic[60]={0};
    int8 pos=0;
    memcpy( msgtopic+pos,"dev2app/",strlen("dev2app/"));
    pos+=strlen( "dev2app/" );
    memcpy( msgtopic+pos,szDID,strlen(szDID) );
    pos+=strlen(szDID);
    msgtopic[pos] = '/';
    pos++;
    memcpy( msgtopic+pos,pPhoneClient,strlen(pPhoneClient));
    pos+=strlen( pPhoneClient );
    msgtopic[pos] = '\0';
    GAgent_Printf( GAGENT_DEBUG,"msgtopic:%s  :len=%d",msgtopic,strlen(msgtopic) );

    //TODO ack buf.
    PubMsg( &g_stMQTTBroker,msgtopic,pData,datalen,0 );
    return ;
}
int32 Mqtt_DispatchPublishPacket( pgcontext pgc,u8 *packetBuffer,int32 packetLen )
{
    u8 topic[128];
    int32 topiclen;
    u8 *pHiP0Data;
    int32 HiP0DataLen;
    int32 i;
    u8 varlen=0;
    u8  clientid[PHONECLIENTID + 1];
    //int32 clientidlen = 0;
    u8 *pTemp;
    u8  DIDBuffer[32];
    u16 cmd;
    u16 *pcmd=NULL;

    topiclen = mqtt_parse_pub_topic(packetBuffer, topic);
    //HiP0DataLen = packetLen - topiclen;
    topic[topiclen] = '\0';

    HiP0DataLen = mqtt_parse_publish_msg(packetBuffer, &pHiP0Data); 


    if(strncmp(topic,"app2dev/",strlen("app2dev/"))==0)
    {
    
        varlen = mqtt_num_rem_len_bytes( pHiP0Data+3 );

        pcmd = (u16*)&pHiP0Data[4+varlen+1];
        cmd = ntohs( *pcmd );  
        pTemp = &topic[strlen("app2dev/")];
        i = 0;
        while (*pTemp != '/')
        {
            DIDBuffer[i] = *pTemp;
            i++;
            pTemp++;
        }
        DIDBuffer[i] = '\0';

        pTemp ++; /* 跳过\/ */
        i=0;
        while (*pTemp != '\0' && i <= PHONECLIENTID)
        {
            clientid[i] = *pTemp;
            i++;
            pTemp++;
        }
        if(i > PHONECLIENTID)
        {
            /* should handle invalid phone client id.don't ack the cmd */
            i = PHONECLIENTID;
        }
        clientid[i]= '\0';
        strcpy( pgc->rtinfo.waninfo.phoneClientId ,clientid );
        memcpy( packetBuffer,pHiP0Data,HiP0DataLen );
        GAgent_Printf( GAGENT_INFO,"Cloud CMD =%04X",cmd );
        if( cmd==0x0093 )
        {
            uint8 tempCmd[2];
            tempCmd[0] = pHiP0Data[4+varlen+1];
            tempCmd[1] = pHiP0Data[4+varlen+1+1];
            pHiP0Data[4+varlen+1]=0x00;
            pHiP0Data[4+varlen+1+1]=0x94;
            //ack to cloud
            Mqtt_Ack2Cloud( pgc->rtinfo.waninfo.phoneClientId,pgc->gc.DID,pHiP0Data,HiP0DataLen );

            pHiP0Data[4+varlen+1] = tempCmd[0];
            pHiP0Data[4+varlen+1+1] = tempCmd[1];
        }
        return HiP0DataLen;
    }
    // 订阅最新固件响应
    else if(strncmp(topic,"ser2cli_res/",strlen("ser2cli_res/"))==0)
    {

        pcmd = (u16*)&pHiP0Data[4];
        cmd = ntohs( *pcmd );        
        // pHiP0Data消息体的指针
        // HiP0DataLen消息体的长度 packetBuffer
        switch(cmd)
        {
            /* V4.1 Don't use this cmd */
            case 0x020e:
                break;
            // wan client on line numbers res.
            case 0x0210:
                Mqtt_ResOnlineClient( pgc,pHiP0Data, HiP0DataLen);
            break;
            case 0x0211:
                //todo MCU OTA.
                GAgent_Printf( GAGENT_DEBUG,"M2M cmd to check OTA!!! ");
                GAgent_SetCloudConfigStatus( pgc,CLOUD_RES_GET_TARGET_FID ); 
            break;
            default:
            break;
        }
        return 0;
    }
    return 0;
}
