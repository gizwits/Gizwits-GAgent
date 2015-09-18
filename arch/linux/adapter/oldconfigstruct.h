/******************************************************************************
 File : oldconfigstruct.h
 说明 : 为了兼容2015-04以前旧版本考虑
        直接拷贝该文件即可
 ******************************************************************************/
#ifndef _OLDCONFIGSTRUCT_H_
#define _OLDCONFIGSTRUCT_H_

typedef struct _JD_INFO_
{
    char product_uuid[32];
    char feed_id[64];
    char access_key[64];
    char ischanged;
    char tobeuploaded;
}old_jd_info;
/* XPG GAgent Global Config data*/
typedef struct _GAGENT_CONFIG
{
    unsigned int magicNumber; //0x12345678
    unsigned int flag;
    char wifipasscode[16]; /* gagent passcode */
    char wifi_ssid[32]; /* WiFi AP SSID */
    char wifi_key[32]; /* AP key */
    char FirmwareId[8];  /* Firmware ID,identity application version */
    char Cloud_DId[24]; /* Device, generate by server, unique for devices */
    char FirmwareVerLen[2];
    char FirmwareVer[32];
    old_jd_info jdinfo;
    char old_did[24];
    char old_wifipasscode[16];
    char old_productkey[33];
    char m2m_ip[17];
    char api_ip[17];
    char airkiss_value; //airkiss BC value  to app.
}GAgent_OldCONFIG_S;

#endif /* endif _OLDCONFIGSTRUCT_H_ */