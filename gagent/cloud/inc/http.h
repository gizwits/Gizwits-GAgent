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

int32 Http_POST( int32 socketid, const int8 *host,const int8 *passcode,const int8 *mac,
                        const uint8 *product_key );
int32 Http_GET( const int8 *host,const int8 *did,int32 socketid );
int32 Http_ReadSocket( int32 socket,int8 *Http_recevieBuf,int32 bufLen );
int32 Http_handlePacket();
int32 Http_Response_DID( uint8 *Http_recevieBuf,int8 *DID );
int32 Http_getdomain_port( uint8 *Http_recevieBuf,int8 *domain, int32 *port );
int32 Http_Sent_Provision();
int32 Http_GetTarget( const int8 *host, 
                    const uint8 *product_key, 
                    const uint8 *did,enum OTATYPE_T type,
                    const uint8 *hard_version, 
                    const uint8 *soft_version,
                    const int32 current_fid,int32 socketid );
int32 Http_JD_Get_uuid_req( const int8 *host,const int8 *product_key );
int32 Http_JD_Post_Feed_Key_req( int32 socketid,int8 *feed_id,int8 *access_key,int8 *DId,int8 *host );
int32 HTTP_DoGetTargetId(enum OTATYPE_T type,const int8 *host,int8 *szDID,uint8 *pk,
                         uint8 *szHver,uint8 *szSver,int32 socketid );
int32 GAgent_DoOTAbyUrl( const int8 *download_url );
int32 Http_Delete( int32 socketid,const int8 *host,const int8 *did,const int8 *passcode);
uint8 Http_Get3rdCloudInfo( int8 *szCloud3Name,int8 *szCloud3Info,uint8 *buf );
#endif // endof __HTTP_H_

