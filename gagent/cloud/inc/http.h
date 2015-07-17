#ifndef __HTTP_H_
#define __HTTP_H_
#include  "gagent_typedef.h"

#define     HTTP_GET_DID            1
#define     HTTP_PROVISION          2
#define     HTTP_GET_TARGET_FID     3
#define     HTTP_GET_JD_UUID        4
#define     HTTP_POST_JD_INFO       5
#define     HTTP_OTA                6
#define     HTTP_DISABLE_DID        7

#define kCRLFNewLine     "\r\n"
#define kCRLFLineEnding  "\r\n\r\n"

int32 Http_POST( int32 socketid, const int8 *host,const int8 *passcode,const int8 *mac,
                        const int8 *product_key );
int32 Http_GET( const int8 *host,const int8 *did,int32 socketid );
int32 Http_GetHost( int8 *downloadurl,int8 **host,int8 **url );
int32 Http_ReadSocket( int32 socket,uint8 *Http_recevieBuf,int32 bufLen );
int32 Http_Response_DID( uint8 *Http_recevieBuf,int8 *DID );
int32 Http_Response_Code( uint8 *Http_recevieBuf );
int32 Http_getdomain_port( uint8 *Http_recevieBuf,int8 *domain, int32 *port );
int32 CheckFirmwareUpgrade(const int8 *host, const int8 *did,enum OTATYPE_T type,
                                  const int8 *passcode,const int8 *hard_version, 
                                  const int8 *soft_version, int32 socketid );
int32 Http_GetSoftver_Url( int8 *download_url, int8 *softver, uint8 *buf );
int32 Http_ReqGetFirmware( int8 *url,int8 *host,int32 socketid );
int32 Http_JD_Get_uuid_req( const int8 *host,const int8 *product_key );
int32 Http_JD_Post_Feed_Key_req( int32 socketid,int8 *feed_id,int8 *access_key,int8 *DId,int8 *host );
int32 GAgent_DoOTAbyUrl( const int8 *download_url );
int32 Http_Delete( int32 socketid,const int8 *host,const int8 *did,const int8 *passcode);
uint8 Http_Get3rdCloudInfo( int8 *szCloud3Name,int8 *szCloud3Info,uint8 *buf );
int32 Http_HeadLen( uint8 *httpbuf );
int32 Http_BodyLen( uint8 *httpbuf );
int32 Http_GetMD5( uint8 *httpbuf,uint8 *MD5,int8 *strMD5);
int32 Http_GetSV(uint8 *httpbuf,int8 *SV );
uint32 GAgent_SaveUpgradFirmware( int offset,uint8 *buf,int len );




#endif // endof __HTTP_H_

