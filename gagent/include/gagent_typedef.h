#ifndef _GAGENT_TYPEDEF_H_H_
#define _GAGENT_TYPEDEF_H_H_ 
#include "platform.h"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef char  int8;
typedef short int16;
typedef int   int32;

typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;


typedef void (*task)(void *arg);

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif /* endof */

#ifndef TRUE
#define TRUE 1
#endif /* endof */

#define IS_BIT_SET(val, bit) (((val) >> (bit)) & (1 >> 0))
//#define SET_BIT(val, bit) ((val) |= (1 << bit))
#define CLEAR_BIT(val, bit) ((val) &= ~(1 << bit))

#define DID_LEN      24
#define PASSCODE_LEN 11
#define PASSCODE_MAXLEN 16
#define PK_LEN       33
#define IP_LEN       17
#define SSID_LEN     32
#define WIFIKEY_LEN  32
#define URL_LEN      512
#define CLOUD3NAME  10

/* use for mqtt var len */
typedef struct _varc
{
    int8 var[2];//the value of mqtt
    int8 varcbty;// 1b-4b B=Bit,b=byte
} varc;

/*  */
typedef struct _jd_info
{
    int8 product_uuid[32];
    int8 feed_id[64];
    int8 access_key[64];
    int8 ischanged;
    int8 tobeuploaded;
}jd_info;


/* OTA 升级类型 */
typedef enum OTATYPE_T
{
    OTATYPE_WIFI = 1,
    OTATYPE_MCU = 2
}OTATYPE;

typedef struct _packet
{
    int32 totalcap;  /* 全局缓存区的大小 4k+128byte */
    int32 remcap;    /* 数据包头的冗余长度 128byte */
    uint8 *allbuf;   /* 数据起始地址 */  
    int32 bufcap;    /* 数据区域的大小 4K */
    
    uint32 type;     /* 消息体类型 */
    uint8 *phead;    /* 数据包头起始地址*/
    uint8 *ppayload;   /* 业务逻辑头起始地址*/
    uint8 *pend;     /* 数据结束地址 */ 
} packet,*ppacket;

typedef struct  _GAgent3Cloud
{
    union
    {
        jd_info jdinfo;
        int8 rsvd[1024];
    };
    int8 cloud3Name[CLOUD3NAME]; /*3rd cloud name*/
}GAgent3Cloud;
/* 需要保存在flash上的数据 */
typedef struct GAGENT_CONFIG
{
    uint32 magicNumber;
    uint32 flag;
    int8 wifipasscode[PASSCODE_MAXLEN]; /* gagent passcode */
    int8 wifi_ssid[SSID_LEN]; /* WiFi AP SSID */
    int8 wifi_key[WIFIKEY_LEN]; /* AP key */
    int8 DID[DID_LEN]; /* Device, generate by server, unique for devices */
    int8 FirmwareVerLen[2];
    int8 FirmwareVer[32];
    
    int8 old_did[DID_LEN];
    int8 old_wifipasscode[PASSCODE_LEN];
    int8 old_productkey[PK_LEN];
    int8 m2m_ip[IP_LEN];
    int8 GServer_ip[IP_LEN];
    int8 airkiss_value; //airkiss BC value to app.
    int8 rsvd[256];
    /** 3rd cloud **/
    GAgent3Cloud cloud3info;
}GAGENT_CONFIG_S, gconfig, *pgconfig;


/* MCU信息 */
typedef struct _XPG_MCU
{
    /* XPG_Ping_McuTick */
    //uint32 XPG_PingTime;
    uint32 oneShotTimeout;

    uint16  passcodeEnableTime;
    uint8 timeoutCnt;
    //int8 loseTime;
    /* 8+1('\0') for print32f. */
    uint8   protocol_ver[8+1];
    uint8   p0_ver[8+1];
    uint8   hard_ver[8+1];
    uint8   soft_ver[8+1];
    uint8   product_key[32+1];
}XPG_MCU;

typedef struct _wifiStatus
{
   int8 wifiStrength;
}wifistatus;
typedef struct _waninfo
{
    uint32 send2HttpLastTime;
    uint32 send2MqttLastTime;
    uint32 RefreshIPLastTime;
    uint32 RefreshIPTime;
    int32 http_socketid;
    int32 m2m_socketid;    
    int32 wanclient_num;

    uint16 CloudStatus;
    uint16 mqttstatus;
    uint16 mqttMsgsubid;

    int8 Cloud3Flag;/* need to connect 3rd cloud flag */
    int8 AirLinkFlag;    
    int8 firmwarever[32];  /* only for store OTA firmware version, not current */
    int8 phoneClientId[32];
    int8 cloudPingTime;
    
}WanInfo;
typedef struct _runtimeinfo3rd
{
    /* 3rd info*/
    uint32 JD_Post_lastTime;
}RunTimeInfo3rd;
typedef struct runtimeinfo_t
{
    //uint32 connect2CloudLastTime;
    uint32 wifistatustime;
    int32 uart_fd;
    uint16 GAgentStatus;/* gagentStatus */
    uint16 lastGAgentStatus;
   
    int8 status_ip_flag; /* when got the ip in station mode,value is 1. */
    uint8 logSwitch[2];
    int8 loglevel;
    wifistatus devWifiStatus;
    
    WanInfo waninfo;
    fd_set readfd;
    ppacket Txbuf;/* send data to local buf */
    ppacket Rxbuf;/* receive data from local buf */
    RunTimeInfo3rd cloud3rd;
}runtimeinfo, *pruntimeinfo;

typedef struct modeinfo_t
{
    int32 m2m_Port;
    int8 m2m_SERVER[128];
    uint8 szmac[16];
    int8 ap_name[32];
}modeinfo, *pmodeinfo;


typedef struct webserver_t
{
    int32 fd;
    int32 nums;
    u8 *buf;
}webserver, *pwebserver;

typedef struct lanserver_t
{
    int32 udpBroadCastServerFd;
    int32 udpServerFd;
    int32 tcpServerFd;
    int32 tcpClientNums;
    uint32 udpSendBroadCastTime;
    struct{
        int32 fd;
        int32 timeout;
        int32 isLogin;
    }tcpClient[8];
    
}lanserver, *planserver;

/* global context, or gagent context */
typedef struct gagentcontext_t
{
    /* modeinfo mi; */
    modeinfo minfo;
    runtimeinfo rtinfo;
    /* mcuinfo mcui; */
    XPG_MCU mcu;
    /* webserver ws; */
    webserver ws;
    /* lanserver ls; */
    lanserver ls;
    /* logman lm; */
    gconfig gc;
}gcontext, *pgcontext;

//typedef int32 (*filter_t)(BufMan*, u8*);
typedef int32 (*pfMasterMCU_ReciveData)( pgcontext pgc, ppacket Rxbuf );
typedef int32  (*pfMasertMCU_SendData)( int serial_fd,unsigned char *buf,int buflen );
#endif
