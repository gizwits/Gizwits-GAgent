#ifndef _GAGENT_H_
#define _GAGENT_H_ 
#include "gagent_typedef.h"
#include "iof_arch.h"
#include "utils.h"

#define GAGENT_RELEASE 1

#define GAGENT_MAGIC_NUM    0x55aa1122

extern pgcontext pgContextData;

#if( GAGENT_RELEASE==1)
  #define HTTP_SERVER         "api.gizwits.com"
#else
  #define HTTP_SERVER         "api.iotsdk.com"
#endif

#define GAGENT_TEST_AP1        "GIZWITS_TEST_1"
#define GAGENT_TEST_AP2        "GIZWITS_TEST_2"
#define GAGENT_TEST_AP_PASS    "GIZWITS_TEST_PASS" 

/*For GAgent Defined SoftAP*/
#define AP_NAME             "XPG-GAgent-"
#define AP_PASSWORD         "123456789"
#define AP_LOCAL_IP         "10.10.100.254"
#define AP_NET_MASK         "255.255.255.0"
#define ADDRESS_POOL_START  "10.10.100.240"
#define ADDRESS_POOL_END    "10.10.100.255"

#define RET_SUCCESS (0)
#define RET_FAILED  (-1)
#define GAGENT_BUF_LEN  1024
#define SOCKET_RECBUFFER_LEN (1*1024)
#define SOCKET_TCPSOCKET_BUFFERSIZE    (1*1024)


/* Macro of module LAN */
#define GAGENT_TCP_SERVER_PORT          12416
#define LAN_UDP_SERVER_PORT             12414
#define LAN_UDP_BROADCAST_SERVER_PORT   2415

#define LAN_TCPCLIENT_MAX           8       /* max 8 tcp socket */
#define LAN_CLIENT_MAXLIVETIME      14      /* 14S,timeout */
#define FIRMWARE_LEN_MAX            32

//add by Frank liu 20150414
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#define SEND_UDP_DATA_TIMES 30

//GAgentStatus
#define WIFI_MODE_AP                  (1<<0)
#define WIFI_MODE_STATION             (1<<1)
#define WIFI_MODE_ONBOARDING          (1<<2)
#define WIFI_MODE_BINDING             (1<<3)
#define WIFI_STATION_CONNECTED        (1<<4)
#define WIFI_CLOUD_CONNECTED          (1<<5)
#define WIFI_LEVEL                    (7<<8)
#define WIFI_CLIENT_ON                (1<<11)
#define WIFI_MODE_TEST                (1<<12)

#define CLOUD_INIT                 1
#define CLOUD_REQ_GET_DID          2
#define CLOUD_RES_GET_DID          3
#define CLOUD_REQ_PROVISION        4
#define CLOUD_RES_PROVISION        5
#define CLOUD_REQ_GET_TARGET_FID   6
#define CLOUD_RES_GET_TARGET_FID   7
#define CLOUD_REQ_OTA              8
#define CLOUD_RES_OTA              9

#define CLOUD_REQ_DISABLE_DID      10
#define CLOUD_RES_DISABLE_DID      11
#define CLOUD_REQ_GET_JD_UUID      12
#define CLOUD_RES_GET_JD_UUID      13
#define CLOUD_REQ_POST_JD_INFO     14
#define CLOUD_RES_POST_JD_INFO     15
#define CLOUD_CONFIG_OK            16

#define LOCAL_DATA_IN     (1<<0)
#define LOCAL_DATA_OUT    (1<<1)
#define LAN_TCP_DATA_IN   (1<<2)
#define LAN_TCP_DATA_OUT  (1<<3)
#define LAN_UDP_DATA_IN   (1<<4)
#define LAN_UDP_DATA_OUT  (1<<5)
#define CLOUD_DATA_IN     (1<<6)
#define CLOUD_DATA_OUT    (1<<7)

/********time define***********/
#define ONE_SECOND  (1)
#define ONE_MINUTE  (60 * ONE_SECOND)
#define ONE_HOUR    (60 * ONE_MINUTE)
#define GAGENT_CLOUDREADD_TIME     10
#define GAGENT_HTTP_TIMEOUT        5//*ONE_SECOND
#define GAGENT_MQTT_TIMEOUT        5//*ONE_SECOND

/*Gizwits heartbeat with eath others, Cloud, SDK/Demo, GAgent and MCU*/
#define MCU_HEARTBEAT           55

#define LOCAL_GAGENTSTATUS_INTERVAL   (10*ONE_MINUTE)

#define CLOUD_HEARTBEAT          50//*ONE_SECOND
#define CLOUD_MQTT_SET_ALIVE          120
//#define HTTP_TIMEOUT            60//*ONE_SECOND
/*broadcasttime(S)*/
#define BROADCAST_TIME          30*ONE_SECOND
/* JD config timeout xs */
#define JD_CONFIG_TIMEOUT       1*ONE_SECOND

/*For V4, GAgent waiting for MCU response of very CMD send by GAgent, Unit: ms*/
#define MCU_ACK_TIME_MS    200

#define GAGENT_CRITICAL    0x00
#define GAGENT_ERROR       0X01
#define GAGENT_WARNING     0X02
#define GAGENT_INFO        0X03
#define GAGENT_DEBUG       0x04
#define GAGENT_DUMP        0x05

/* GAgent_CONFIG_S flag */
#define XPG_CFG_FLAG_CONNECTED      (1<<0) /* 重要。wifi工作模式。1=STA，0=AP */
#define XPG_CFG_FLAG_CHANGEPW       (1<<1) /* 需要修改passcode */
#define XPG_CFG_FLAG_CONFIG         (1<<2) /* 重要。是否使用Onboarding或者AirLink配置过 */
#define XPG_CFG_FLAG_CONFIGSUCCESS  (1<<3) /* 使用外部配置方式是否成功。需要用来进行上电宣告。注意和CONFIG的区别 */
#define CFG_FLAG_TESTMODE           (1<<4)
#define XPG_CFG_FLAG_CONFIG_AIRKISS (1<<5) /* 平台用来标示是否airkiss配置过 需要发送airkiss配置包*/

#define ERRORCODE GAgent_Printf(GAGENT_ERROR,"%s %d",__FUNCTION__,__LINE__ );

/*V4 CMD of P0*/
#define MCU_INFO_CMD        0X01
#define MCU_INFO_CMD_ACK    0X02

#define MCU_CTRL_CMD        0X03
#define MCU_CTRL_CMD_ACK    0X04

#define MCU_REPORT          0X05
#define MCU_REPORT_ACK      0X06

#define WIFI_PING2MCU       0X07
#define WIFI_PING2MCU_ACK   0X08

#define MCU_CONFIG_WIFI     0X09
#define MCU_CONFIG_WIFI_ACK 0X0A

#define MCU_RESET_WIFI      0X0B
#define MCU_RESET_WIFI_ACK  0X0C

#define WIFI_STATUS2MCU     0X0D
#define WIFI_STATUS2MCU_ACK 0X0E

#define MCU_DATA_ILLEGAL    0x11
#define MCU_REPLY_GAGENT_DATA_ILLEGAL    0x12

#define WIFI_TEST           0x13
#define WIFI_TEST_ACK       0x14

#define MCU_ENABLE_BIND     0x15
#define MCU_ENABLE_BIND_ACK 0x16

#define MCU_LEN_POS            2
#define MCU_CMD_POS            4
#define MCU_SN_POS             5
#define MCU_FLAG_POS           6
#define MCU_ERROR_POS          8
#define MCU_HDR_LEN            8
#define MCU_LEN_NO_PAYLOAD     9
#define MCU_HDR_FF             0xFF
#define MCU_P0_POS             8
#define MCU_BYTES_NO_SUM        3
#define LOCAL_GAGENTSTATUS_MASK 0x1FFF

#define BUF_LEN 1024*4      /* depend on you platform ram size */
#define BUF_HEADLEN 128

#define AS xpg_assert(0) // 显示代码位置
#define TS (GAgent_Printf(GAGENT_INFO, __FILE__ ":" _STR(__LINE__) ":" _STR(__FUNC__) "Flag:%x,MN:%x ...", g_stGAgentConfigData.flag, g_stGAgentConfigData.magicNumber))
#define MS (GAgent_Printf(GAGENT_INFO, __FILE__ ":" _STR(__LINE__) ":" _STR(__FUNC__) "TOP Level Error:malloc")) // 内存分配错误
#define MBM ((void)0) // 标记，需要注意的地方

void GAgent_Init( pgcontext *pgc );
void GAgent_VarInit( pgcontext *pgc );
void GAgent_WiFiInit( pgcontext pgc );
void GAgent_dumpInfo( pgcontext pgc );
/********************************************** GAgent Cloud API **********************************************/
int32 GAgent_Cloud_GetPacket( pgcontext pgc,ppacket pbuf , int32 buflen);
void GAgent_Cloud_Handle( pgcontext pgc, ppacket Rxbuf,int32 length );
uint32 GAgent_Cloud_SendData( pgcontext pgc,ppacket pbuf, int32 buflen );
uint32 GAgent_Cloud_OTAByUrl( int32 socketid,uint8 *downloadUrl );
uint32 GAgent_Cloud_Disconnect();
/********************************************** GAgent Lan API **********************************************/
uint32 GAgent_Lan_Handle(pgcontext pgc, ppacket prxBuf, ppacket ptxBuf, int32 len);
uint32 GAgent_Lan_SendData();
void GAgent_DoTcpWebConfig( pgcontext pgc );
uint32 GAgent_Exit();

/********************************************** GAgent Local API **********************************************/
/* GAgent receive data hook function */
void GAgent_RegisterReceiveDataHook( pfMasterMCU_ReciveData fun );
/* GAgent send data hook function */
void GAgent_RegisterSendDataHook( pfMasertMCU_SendData fun );
void GAgent_LocalInit( pgcontext pgc );
void GAgent_LocalSendGAgentstatus(pgcontext pgc,uint32 dTime_s );
int32 GAgent_Local_GetPacket( pgcontext pgc, ppacket Rxbuf );
int32 GAgent_LocalDataWriteP0( pgcontext pgc,int32 fd,ppacket pTxBuf,uint8 cmd );
void GAgent_Local_Handle( pgcontext pgc,ppacket Rxbuf,int32 length );


void GAgent_logevelSet( uint16 level );
void GAgent_SetWiFiStatus( pgcontext pgc,uint16 wifistatus,int8 flag );
void GAgent_SetCloudConfigStatus( pgcontext pgc,int16 cloudstauts );
void GAgent_SetCloudServerStatus( pgcontext pgc,int16 serverstatus );
int8 GAgent_SetGServerIP( pgcontext pgc,int8 *szIP );
int8 GAgent_SetGServerSocket( pgcontext pgc,int32 socketid );
uint8 GAgent_SetDeviceID( pgcontext pgc,int8 *deviceID );
int8 GAgent_IsNeedDisableDID( pgcontext pgc );
void GAgent_Reset( pgcontext pgc );
void GAgent_Config( uint8 typed,pgcontext pgc );
int8 GAgent_SetOldDeviceID( pgcontext pgc,int8* p_szDeviceID,int8* p_szPassCode,int8 flag );
void GAgent_UpdateInfo( pgcontext pgc,int8 *new_pk );
void  GAgent_AddSelectFD( pgcontext pgc );
int32 GAgent_MaxFd( pgcontext pgc ) ;
int8 GAgent_loglevelenable( uint16 level );
void GAgentSetLedStatus( uint16 gagentWiFiStatus );

uint8 GAgent_EnterTest( pgcontext pgc );
uint8 GAgent_ExitTest( pgcontext pgc );

uint32 GAgent_BaseTick();
void GAgent_Tick( pgcontext pgc );
void GAgent_CloudTick( pgcontext pgc,uint32 dTime_s);
void GAgent_LocalTick( pgcontext pgc,uint32 dTime_s );
void GAgent_LanTick( pgcontext pgc,uint32 dTime_s );
void GAgent_RefreshIPTick( pgcontext pgc,uint32 dTime_s );
void GAgent_WiFiEventTick( pgcontext pgc,uint32 dTime_s );


#endif