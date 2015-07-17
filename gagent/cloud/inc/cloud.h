#ifndef _CLOUD_H_
#define _CLOUD_H_

#define MQTT_STATUS_START               1
#define MQTT_STATUS_REQ_LOGIN           2
#define MQTT_STATUS_RES_LOGIN           3
#define MQTT_STATUS_REQ_LOGINTOPIC1     4
#define MQTT_STATUS_RES_LOGINTOPIC1     5
#define MQTT_STATUS_REQ_LOGINTOPIC2     6
#define MQTT_STATUS_RES_LOGINTOPIC2     7
#define MQTT_STATUS_REQ_LOGINTOPIC3     8
#define MQTT_STATUS_RES_LOGINTOPIC3     9
#define MQTT_STATUS_RUNNING             10

int32 GAgent_MCUOTAByUrl( pgcontext pgc,int8 *szdownloadUrl );
uint32 GAgent_ReqServerTime(pgcontext pgc);
uint32 GAgent_Get_Gserver_Time(uint32 *clock,uint8 *Http_recevieBuf,int32 respondCode);
int32 Cloud_InitSocket( int32 iSocketId,int8 *p_szServerIPAddr,int32 port,int8 flag );

uint32 Cloud_ReqRegister( pgcontext pgc );
int32 Cloud_ResRegister( uint8 *cloudConfiRxbuf,int32 buflen,int8 *pDID,int32 respondCode );
uint32 Cloud_ReqGetFid( pgcontext pgc,enum OTATYPE_T type );
int8 Cloud_ResGetFid( int8 *download_url, int8 *fwver, uint8 *cloudConfiRxbuf,int32 respondCode );
uint32 Cloud_ReqProvision( pgcontext pgc );
uint32 Cloud_ResProvision( int8 *szdomain,int32 *port,uint8 *cloudConfiRxbuf,int32 respondCode );
uint32 Cloud_ReqConnect( pgcontext pgc,const int8 *username,const int8 *password );
uint32 Cloud_ResConnect( uint8* buf );
uint32 Cloud_ReqSubTopic( pgcontext pgc,uint16 mqttstatus );
uint32 Cloud_ResSubTopic( const uint8* buf,int8 msgsubId );
uint32 Cloud_ReqDisable( pgcontext pgc );
uint32 Cloud_ResDisable( int32 respondCode );
uint32 Cloud_JD_Post_ReqFeed_Key( pgcontext pgc );
uint32 Cloud_JD_Post_ResFeed_Key( pgcontext pgc,int32 respondCode );

int32 Cloud_ReadGServerConfigData( pgcontext pgc ,int32 socket,uint8 *buf,int32 buflen );
uint32 Cloud_ConfigDataHandle( pgcontext pgc );
int32 Cloud_M2MDataHandle(  pgcontext pgc,ppacket pbuf /*, ppacket poutBuf*/, uint32 buflen);
uint32 Cloud_isNeedOTA( pgcontext pgc, int type, int8 *sFV );
void Log2Cloud(pgcontext pgc);
void Cloud_ClearClientAttrs(pgcontext pgc, stCloudAttrs_t *client);
void Cloud_SetClientAttrs(pgcontext pgc, uint8 *clientid, uint16 cmd, int32 sn);

#endif

