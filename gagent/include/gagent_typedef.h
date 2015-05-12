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
#define PASSCODE_LEN 10
#define PASSCODE_MAXLEN 32
#define PK_LEN       32
#define IP_LEN_MAX   15         /* eg:192.168.180.180 15byte.Should be ended with '\0' */
#define IP_LEN_MIN   7          /* eg:1.1.1.1 7bytes. Should be ended with '\0' */
#define SSID_LEN_MAX     32
#define WIFIKEY_LEN_MAX  32
#define CLOUD3NAME   10
#define FIRMWARELEN  32
#define MAC_LEN      32
#define APNAME_LEN   32
#define M2MSERVER_LEN 128
#define PHONECLIENTID 23
#define PRODUCTUUID_LEN   32
#define FEEDID_LEN 64
#define ACCESSKEY_LEN  64
#define MCU_P0_LEN 2
#define MCU_CMD_LEN 2
#define MCU_PROTOCOLVER_LEN 8
#define MCU_P0VER_LEN 8
#define MCU_HARDVER_LEN 8
#define MCU_SOFTVER_LEN 8
#define MCU_MCUATTR_LEN 8

/* use for mqtt var len */
typedef struct _varc
{
    int8 var[2];//the value of mqtt
    int8 varcbty;// 1b-4b B=Bit,b=byte
} varc;

/*  */
typedef struct _jd_info
{
    int8 product_uuid[PRODUCTUUID_LEN+1];
    int8 feed_id[FEEDID_LEN+1];
    int8 access_key[ACCESSKEY_LEN+1];
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
    int8 wifipasscode[PASSCODE_MAXLEN+1]; /* gagent passcode +1 for printf*/
    int8 wifi_ssid[SSID_LEN_MAX+1]; /* WiFi AP SSID */
    int8 wifi_key[WIFIKEY_LEN_MAX+1]; /* AP key */
    int8 DID[DID_LEN]; /* Device, generate by server, unique for devices */
    int8 FirmwareVerLen[2];
    int8 FirmwareVer[FIRMWARELEN+1];
    
    int8 old_did[DID_LEN];
    int8 old_wifipasscode[PASSCODE_MAXLEN + 1];
    int8 old_productkey[PK_LEN + 1];    /* Add 1byte '\0' */
    int8 m2m_ip[IP_LEN_MAX + 1];        /* Add 1byte '\0' */
    int8 GServer_ip[IP_LEN_MAX + 1];    /* Add 1byte '\0' */
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
	uint16  passcodeTimeout;
    uint8 timeoutCnt;
    //int8 loseTime;
    /* 8+1('\0') for print32f. */
    uint8   protocol_ver[MCU_PROTOCOLVER_LEN+1];
    uint8   p0_ver[MCU_P0VER_LEN+1];
    uint8   hard_ver[MCU_HARDVER_LEN+1];
    uint8   soft_ver[MCU_SOFTVER_LEN+1];
    uint8   product_key[PK_LEN+1];
    uint8   mcu_attr[MCU_MCUATTR_LEN];
}XPG_MCU;

typedef struct _wifiStatus
{
   int8 wifiStrength; //WiFi信号强度.
}wifistatus;
typedef struct _waninfo
{
    uint32 send2HttpLastTime;
    uint32 send2MqttLastTime;
    uint32 ReConnectMqttTime;
    uint32 ReConnectHttpTime;
    uint32 firstConnectHttpTime;
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
    int8 phoneClientId[PHONECLIENTID+1];
    int8 cloudPingTime;
    int8 httpCloudPingTime;
}WanInfo;
typedef struct _runtimeinfo3rd
{
    /* 3rd info*/
    uint32 JD_Post_lastTime;
}RunTimeInfo3rd;
typedef struct _localmodule
{
    uint8 timeoutCnt;
    uint32 oneShotTimeout;
    int32 uart_fd;

}localmodule;
typedef struct runtimeinfo_t
{
    uint32 wifistatustime;
    uint32 updatestatusinterval;
    uint32 testLastTimeStamp;

    uint16 GAgentStatus;/* gagentStatus */
    uint16 lastGAgentStatus;
   
    int8 status_ip_flag; /* when got the ip in station mode,value is 1. */
    uint8 logSwitch[2];
    int8 loglevel;
    uint8 scanWifiFlag;
    int8 firstStartUp;

    wifistatus devWifiStatus;
    
    WanInfo waninfo;
    localmodule local;
    fd_set readfd;
    ppacket Txbuf;/* send data to local buf */
    ppacket Rxbuf;/* receive data from local buf */
    RunTimeInfo3rd cloud3rd;
	
}runtimeinfo, *pruntimeinfo;

typedef struct modeinfo_t
{
    int32 m2m_Port;
    int8 m2m_SERVER[M2MSERVER_LEN+1];
    uint8 szmac[MAC_LEN+1];
    int8 ap_name[APNAME_LEN+1];
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
    int32 tcpWebConfigFd;
    int32 tcpClientNums;
    uint32 onboardingBroadCastTime;//config success broadcast Counter
	uint32 startupBroadCastTime;//first on ele broadcast Counter
    int32 broResourceNum;//public resource counter for broadcast
	
    struct{
        int32 fd;
        int32 timeout;
        int32 isLogin;
    }tcpClient[8];
    struct sockaddr_t addr;
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

typedef struct _ApList_str
{
     uint8 ssid[SSID_LEN_MAX+1];
     uint8 ApPower; /* 0-100 min:0;max:100 */
}ApList_str;
typedef struct _NetHostList_str
{
     uint8 ApNum;
     ApList_str *ApList;
}NetHostList_str;

//typedef int32 (*filter_t)(BufMan*, u8*);
typedef int32 (*pfMasterMCU_ReciveData)( pgcontext pgc, ppacket Rxbuf );
typedef int32  (*pfMasertMCU_SendData)( int serial_fd,unsigned char *buf,int buflen );
#endif
