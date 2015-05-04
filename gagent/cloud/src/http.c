#include "gagent.h"
#include "http.h"
#include "string.h"
extern int32 g_MQTTStatus;

#define kCRLFNewLine     "\r\n"
#define kCRLFLineEnding  "\r\n\r\n"


int32 Http_POST( int32 socketid, const int8 *host,const int8 *passcode,const int8 *mac,const uint8 *product_key )
{
    int32 ret=0;
    uint8 *postBuf=NULL;
    int8 *url = "/dev/devices"; 
    int8 Content[100]={0};
    int32 ContentLen=0;
    int32 totalLen=0;
    int8 *contentType="application/x-www-form-urlencoded";
    
    postBuf = (uint8*)malloc(400);
    if (postBuf==NULL) return 1;
    //g_globalvar.http_sockettype =HTTP_GET_DID;//http_sockettype=1 :http_post type.

    sprintf(Content,"passcode=%s&mac=%s&product_key=%s",passcode,mac,product_key);
    ContentLen=strlen(Content);
    snprintf( postBuf,400,"%s %s %s%s%s %s%s%s %d%s%s%s%s%s",
              "POST" ,url,"HTTP/1.1",kCRLFNewLine,
              "Host:",host,kCRLFNewLine,
              "Content-Length:",ContentLen,kCRLFNewLine,
              "Content-Type: application/x-www-form-urlencoded",kCRLFNewLine,
              kCRLFNewLine,
              Content
        );
    totalLen = strlen( postBuf );
    GAgent_Printf(GAGENT_DEBUG,"http_post:%s %d",postBuf,totalLen);    
    ret = send( socketid,postBuf,totalLen,0 );
    GAgent_Printf(GAGENT_DEBUG,"http_post ret: %d",ret);    
    free( postBuf );
    return 0;
}

int32 Http_GET( const int8 *host,const int8 *did,int32 socketid )
{
    static int8 *getBuf=NULL;
    int32 totalLen=0;
    int32 ret=0;
    int8 *url = "/dev/devices/";

    getBuf = (int8*)malloc( 200 );
    if(getBuf == NULL)
    {
        return 1;
    }
    memset( getBuf,0,200 );
    //g_globalvar.http_sockettype =HTTP_PROVISION;//http get type.
    snprintf( getBuf,200,"%s %s%s %s%s%s %s%s%s%s%s",
              "GET",url,did,"HTTP/1.1",kCRLFNewLine,
              "Host:",host,kCRLFNewLine
              "Cache-Control: no-cache",kCRLFNewLine,
              "Content-Type: application/x-www-form-urlencoded",kCRLFLineEnding);
    totalLen =strlen( getBuf );
    ret = send( socketid, getBuf,totalLen,0 );
    GAgent_Printf(GAGENT_DEBUG,"Sent provision:\n %s\n", getBuf);
    free(getBuf);
    getBuf = NULL;

    if(ret<=0 ) 
    {
        return 1;
    }
    else
    {
        return 0;
    }    
}
/******************************************************
 *
 *   FUNCTION        :   delete device did by http 
 *     
 *   return          :   0--send successful 
 *                       1--fail. 
 *   Add by Alex lin  --2014-12-16
 *
 ********************************************************/
int32 Http_Delete( int32 socketid, const int8 *host,const int8 *did,const int8 *passcode )
{
    int32 ret=0;
    int8 *sendBuf=NULL;
    int8 *url = "/dev/devices"; 
    int8 *Content = NULL;
    int32 ContentLen=0;
    int32 totalLen=0;
    int8 *DELETE=NULL;
    int8 *HOST=NULL;
    int8 Content_Length[20]={0};
    int8 *contentType="Content-Type: application/x-www-form-urlencoded\r\n\r\n";

    DELETE = (int8*)malloc(strlen("DELETE  HTTP/1.1\r\n")+strlen(url)+1);//+1 for sprintf
    if( DELETE ==NULL ) 
    {
        return 1;
    }
    HOST = (int8*)malloc(strlen("Host: \r\n")+strlen(host)+1);// +1 for sprintf
    if( HOST==NULL)
    {
      free(DELETE);
      return 1;
    }
    Content = (int8*)malloc(strlen("did=&passcode=")+strlen(did)+strlen(passcode)+1);// +1 for sprintf
    if( Content==NULL )
    {
      free(DELETE);
      free(HOST);
      return 1;      
    }

    sprintf(Content,"did=%s&passcode=%s",did,passcode);
    ContentLen=strlen(Content); 
    sprintf(DELETE,"DELETE %s HTTP/1.1\r\n",url);
    sprintf(HOST,"Host: %s\r\n",host);
    sprintf(Content_Length,"Content-Length: %d\r\n",ContentLen);
    sendBuf = (int8*)malloc(strlen(DELETE)+strlen(HOST)+strlen(Content_Length)+strlen(contentType)+ContentLen+1);//+1 for sprintf
    if (sendBuf==NULL) 
    {
      free(DELETE);
      free(HOST);
      free(Content);
      return 1;
    }
    sprintf(sendBuf,"%s%s%s%s%s",DELETE,HOST,Content_Length,contentType,Content);
    totalLen = strlen(sendBuf);
    ret = send( socketid, sendBuf,totalLen,0 );
    if(ret<=0)
    {
      GAgent_Printf(GAGENT_ERROR," send fail %s %s %d",__FILE__,__FUNCTION__,__LINE__);
      return 1;
    }
    
    GAgent_Printf( GAGENT_DEBUG , "totalLen = %d",totalLen);
    GAgent_Printf(GAGENT_DEBUG,"%s",sendBuf);

    free(DELETE);
    free(HOST);
    free(Content);
    free(sendBuf);
    return 0;
}
/******************************************************
        functionname    :   Http_ReadSocket
        description     :   read data form socket
        socket          :   http server socket
        Http_recevieBuf :   data buf.
        bufLen          :   read data length 
        return          :   >0 the data length read form 
                            socket 
                            <0 error,and need to close 
                            the socket.
******************************************************/
int32 Http_ReadSocket( int32 socket,int8 *Http_recevieBuf,int32 bufLen )
{
    fd_set readfds;
    int32 i=0;
    int32 bytes_rcvd = 0; 
    if( socket<=0 )
        return bytes_rcvd;
    memset(Http_recevieBuf, 0, bufLen);  

    bytes_rcvd = recv(socket, Http_recevieBuf, bufLen, 0 );
    if(bytes_rcvd <= 0)
    {
        GAgent_Printf(GAGENT_DEBUG,"Close HTTP Socket[%d].", socket);
        MBM;
        return -2;
    }
    return bytes_rcvd;
}
/******************************************************
 *
 *   Http_recevieBuf :   http receive data
 *   return          :   http response code
 *   Add by Alex lin  --2014-09-11
 *
 ********************************************************/
int32 Http_Response_Code( uint8 *Http_recevieBuf )
{
    int32 response_code=0;
    int8* p_start = NULL;
    int8* p_end =NULL; 
    int8 re_code[10] ={0};
    memset(re_code,0,sizeof(re_code));

    p_start = strstr( Http_recevieBuf," " );
    if(NULL == p_start)
    {
        return RET_FAILED;
    }
    p_end = strstr( ++p_start," " );
    if(p_end)
    {
        if(p_end - p_start > sizeof(re_code))
        {
            return RET_FAILED;
        }
        memcpy( re_code,p_start,(p_end-p_start) );
    }
    
    response_code = atoi(re_code); 
    return response_code;
}
int32 Http_Response_DID( uint8 *Http_recevieBuf,int8 *DID )
{
    int8 *p_start = NULL;
    int8 *p_end =NULL;
    memset(DID,0,DID_LEN);
    p_start = strstr( Http_recevieBuf,"did=");
    if( p_start==NULL )
        return 1;
    p_start = p_start+strlen("did=");
    memcpy(DID,p_start,DID_LEN);
    DID[DID_LEN - 2] ='\0';             
    return 0;    
}
int32 Http_getdomain_port( uint8 *Http_recevieBuf,int8 *domain,int32 *port )
{
    int8 *p_start = NULL;
    int8 *p_end =NULL;
    int8 Temp_port[10]={0};
    memset( domain,0,100 );
    p_start = strstr( Http_recevieBuf,"host=");
    if( p_start==NULL ) return 1;
    p_start = p_start+strlen("host=");
    p_end = strstr(p_start,"&");
    if( p_end==NULL )   return 1;
    memcpy( domain,p_start,( p_end-p_start ));
    domain[p_end-p_start] = '\0';
    p_start = strstr((++p_end),"port=");
    if( p_start==NULL ) return 1;
    p_start =p_start+strlen("port=");
    p_end = strstr( p_start,"&" ); 
    memcpy(Temp_port,p_start,( p_end-p_start));
    *port = atoi(Temp_port);
    return 0;
}
/******************************************************
 *   FUNCTION       :   sent the HTTP Get target data
 *                      to http server.
 *   host           :   http server host
 *   type           :   1-wifi ,2-mcu
 *   Add by Alex lin  --2014-10-29
 *
 ********************************************************/
int32 Http_GetTarget( const int8 *host, 
                      const uint8 *product_key, 
                      const uint8 *did,enum OTATYPE_T type,
                      const uint8 *hard_version, 
                      const uint8 *soft_version,
                      const int32 current_fid,int32 socketid )
{
    //int8 getBuf[500] = {0};
    int8 *getBuf=NULL;
    int32 totalLen=0;
    int32 ret=0;
    int8 *url = "/dev/ota/target_fid?";
    ;
    
    getBuf = (int8*)malloc(500);
    if(getBuf == NULL)
    {
        return 1;
    }
    
    
    memset( getBuf,0,500 );

    sprintf( getBuf,"%s %s%s%s%s%s%s%d%s%s%s%s%s%d%s%s%s%s%s%s%s",
             "GET",url,"did=",did,"&product_key=",product_key,
             "&type=",type,"&hard_version=",hard_version,
             "&soft_version=",soft_version,
             "&current_fid=",current_fid," HTTP/1.1",kCRLFNewLine,
             "Host: ",host,kCRLFNewLine,
             "Content-Type: application/text",kCRLFLineEnding );

    totalLen =strlen( getBuf );
    GAgent_Printf(GAGENT_INFO,"totalLen=%d\r\n",totalLen);
    ret = send( socketid, getBuf,totalLen,0 );
    free(getBuf);
    if(ret>0) 
        return 0;

    return 1;
}
/******************************************************
 *  FUNCTION  :   Http send the get target id and url
 *                to http server.
 *  OTATYPE_T :   OTATYPE_WIFI or OTATYPE_MCU
 *  szHver    :   it's depend on OTATYPE_T
 *  szSver    :   it's depend on OTATYPE_T
 *  return    :  0-send ok, 1-send fail. 
 *   Add by Alex lin  --2014-10-29
 *
 ********************************************************/
int32 HTTP_DoGetTargetId(enum OTATYPE_T type,const int8 *host,int8 *szDID,uint8 *szPK,uint8 *szHver,
                         uint8 *szSver,int32 socketid )
{
    int32 ret=0, otatype, targetid = 1;
    int32 temp_fid=0;
 
    ret = Http_GetTarget( host,szPK,szDID,type,szHver,szSver,temp_fid,socketid );
    return 0;
}
/******************************************************
 *FUNCTION      :   get the http return target_fid 
 *                  and download url(for hf wifi)
 *  target_fid  :   target_fid
 *  download_url:   download_url 
 *          buf :   http receive data.
 *       return :  0-return ok, 1-return fail. 
 *   Add by Alex lin  --2014-10-29
 *
 ********************************************************/
int32 Http_GetFid_Url( int32 *target_fid,int8 *download_url, int8 *fwver, int8 *buf )
{
    int8 *p_start = NULL;
    int8 *p_end =NULL;
    int8 fid[10]={0};
    int16 fwverlen = 0;
    p_start = strstr( buf,"target_fid=");
    if( p_start==NULL )  return 1;
    p_start =  p_start+(sizeof( "target_fid=" )-1);

    p_end = strstr( p_start,"&");
    if( p_end==NULL )  return 1;
    memcpy( fid,p_start,(p_end-p_start));

    *target_fid = atoi(fid);
    p_start = strstr(p_end,"download_url=");

    if(p_start==NULL ) return 1;
    p_start+=sizeof("download_url=")-1;

    // end with & or /r/n
    memset(fwver, 0x0, 32);
    p_end = strstr( p_start,"&");
    if( p_end==NULL )
    {
        p_end = strstr(p_start,kCRLFNewLine);
        if( p_end==NULL )   return 1;
    }
    memcpy( download_url,p_start,(p_end-p_start));
    download_url[p_end-p_start] = '\0';

    p_start = strstr(p_end,"firmware_version=");
    if(p_start!=NULL )
    {
        p_start+=sizeof("download_url=")-1;

        p_end = strstr(p_start,kCRLFNewLine);
        if( p_end==NULL )   return 1;
        fwverlen = p_end - p_start;
        if(fwverlen > 32)
        {
            fwverlen = 32;
        }
        memcpy(fwver,p_start, fwverlen);
    }
    return 0;
}
/******************************************************
 *
 *   FUNCTION       :   get the uuid by productkey in HTTP GET
 *      productkey  :   productkey
 *      host        :   host
 *      return      :   0-OK 1-fail
 *   Add by Alex lin  --2014-11-10
 *
 ********************************************************/
int32 Http_JD_Get_uuid_req( const int8 *host,const int8 *product_key )
{
    int8 *getBuf=NULL;
    int32 totalLen=0;
    int32 ret=0;
    int8 *url = "/dev/jd/product/";    
    if( strlen(product_key)<=0 )
    {   
        GAgent_Printf(GAGENT_ERROR,"need a productkey to get uuid! ");
        return 1;
    }
    getBuf = (int8*)malloc(500);
    if (getBuf==NULL)  return 1;
    memset( getBuf,0,500 ); 
    //g_globalvar.http_sockettype =HTTP_GET_JD_UUID;//http get product_uuid for JD.

    sprintf( getBuf,"%s %s%s%s%s%s%s%s%s%s",
             "GET",url,product_key," HTTP/1.1",kCRLFNewLine,
             "Host: ",host,kCRLFNewLine,
             "Cache-Control: no-cache",kCRLFLineEnding );

    totalLen =strlen( getBuf );
    GAgent_Printf(GAGENT_DEBUG,"totalLen=%d\r\n",totalLen);
    GAgent_Printf(GAGENT_DEBUG,"Sent Http to get JD uuid.\n");
    GAgent_Printf(GAGENT_DUMP,"%s",getBuf );
    free(getBuf);
   
    if(ret>0) return 0; 
    else    return 1;
}

/******************************************************
 *
 *   FUNCTION           :       post the feedid and accesskey to http server
 *      feedid          :       feedid
 *      accesskey       :       accesskey
 *      return          :
 *   Add by Alex lin  --2014-11-07
 *
 ********************************************************/
int32 Http_JD_Post_Feed_Key_req( int32 socketid,int8 *feed_id,int8 *access_key,int8 *DId,int8 *host )
{
    int32 ret=0;
    uint8 *postBuf=NULL;
    int8 *url = "/dev/jd/"; 
    int8 Content[200]={0};
    int32 ContentLen=0;
    int32 totalLen=0;
    int8 *contentType="application/x-www-form-urlencoded";
    
    if(strlen(DId)<=0)
        return 1;

    postBuf = (uint8*)malloc(400);
    if (postBuf==NULL)
        return 1;
    sprintf(Content,"feed_id=%s&access_key=%s",feed_id,access_key);
    ContentLen = strlen( Content );

    snprintf( postBuf,400,"%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s",
              "POST ",url,DId," HTTP/1.1",kCRLFNewLine,
              "Host: ",host,kCRLFNewLine,
              "Cache-Control: no-cache",kCRLFNewLine,
              "Content-Type: application/x-www-form-urlencoded",kCRLFNewLine,
              "Content-Length: ",ContentLen,kCRLFLineEnding,
              Content,kCRLFLineEnding                           
        );
    totalLen = strlen( postBuf );
    ret = send( socketid,postBuf,totalLen,0 );
    
    free( postBuf );    
    return 0;       
}
///*********************************************************************
// *
// *   FUNCTION           :    GAgent OTA by url OTA success will reboot
// *   download_url        :    download url 
// *   return             :     1-OTA fail.
// *   Add by Alex lin  --2014-12-01
// *
// **********************************************************************/
//int32 GAgent_DoOTAbyUrl( const int8 *download_url )
//{
//    int8 *GetBuf=NULL;
//    int8 *p_start = NULL;
//    int8 *p_end =NULL;
//    int32 GetBufLen=0,ret=0;
//    int8 url[100]={0};
//    int8 GET[100]={0};
//    int8 HOST[100]={0};
//    int8 *Type="Content-Type: application/text\r\n";
//    int8 *Control="Cache-Control: no-cache\r\n\r\n";

//    GetBuf = (int8*)malloc(500);
//    if( GetBuf==NULL)
//    {
//        GAgent_Printf(GAGENT_INFO," malloc fail %s %s %d",__FILE__,__FUNCTION__,__LINE__);
//        AS;
//        return 1;
//    }
//    memset(GetBuf,0,500);

//    p_start = strstr( download_url,"/dev/ota/");
//    if( p_start==NULL ) return 1;
//    p_end = strstr(download_url,".bin");
//    if( p_end==NULL )       return 1;
//    p_end=p_end+strlen(".bin");
//    memcpy( url,p_start,(p_end-p_start));
//    url[ (p_end-p_start)] ='\0';

//    sprintf(GET,"GET %s HTTP/1.1%s",url,kCRLFNewLine);
//    sprintf(HOST,"Host: %s%s",HTTP_SERVER,kCRLFNewLine);
//    sprintf( GetBuf,"%s%s%s%s",GET,HOST,Type,Control);

//    GetBufLen = strlen(GetBuf);

//    g_globalvar.waninfo.send2HttpLastTime = DRV_GAgent_GetTime_S();
//    g_globalvar.connect2Cloud=0; 
//    ret = Http_InitSocket(1);
//    ret = send( g_globalvar.waninfo.http_socketid, GetBuf,GetBufLen,0 );
//    g_globalvar.http_sockettype=HTTP_OTA;

//    free(GetBuf);
//}

/*********************************************************************
*
*   FUNCTION       :   TO get the http headlen
*     httpbuf      :   http receive buf 
*     return       :   the http headlen.
*   Add by Alex lin  --2014-12-02
*
**********************************************************************/
int32 Http_HeadLen( int8 *httpbuf )
{
   int8 *p_start = NULL;
   int8 *p_end =NULL;
   int8 temp;
   int32 headlen=0;
   p_start = httpbuf;
   p_end = strstr( httpbuf,kCRLFLineEnding);
   if( p_end==NULL )
   {
       GAgent_Printf(GAGENT_DEBUG,"Can't not find the http head!");
       return 0;
   }
   p_end=p_end+strlen(kCRLFLineEnding);
   headlen = (p_end-p_start);
   return headlen;
}
/*********************************************************************
*
*   FUNCTION       :   TO get the http bodylen
*      httpbuf     :   http receive buf 
*      return      :   the http bodylen.(0-error)
*   Add by Alex lin  --2014-12-02
*
**********************************************************************/
int32 Http_BodyLen( int8 *httpbuf )
{
   int8 *p_start = NULL;
   int8 *p_end =NULL;
   int8 temp;
   int8 bodyLenbuf[10]={0};
   int32 bodylen=0;  //Content-Length: 
   p_start = strstr( httpbuf,"Content-Length: ");
   if( p_start==NULL ) return 0;
   p_start = p_start+strlen("Content-Length: ");
   p_end = strstr( p_start,kCRLFNewLine);
   if( p_end==NULL )   return 0;

   memcpy( bodyLenbuf,p_start,(p_end-p_start));
   bodylen = atoi(bodyLenbuf);
   return bodylen;
}
int32 Http_GetFV(int8 *httpbuf,int8 *FV )
{
//Firmware-Version: 
   int8 *p_start = NULL;
   int8 *p_end =NULL;
   int32 FV_Len=0;
   p_start = strstr( httpbuf,"Firmware-Version: ");
   if(p_start==NULL) 
       return 0;
   p_start = p_start+strlen("Firmware-Version: ");
   p_end = strstr( p_start,kCRLFNewLine);
   if(p_end==NULL)
       return 0;
   FV_Len = (p_end-p_start);
   memcpy(FV,p_start,FV_Len);
   FV[FV_Len]='\0';
   return FV_Len;

}
/*************************************************************
*
*   FUNCTION  :  get MD5 from http head.
*   httpbuf   :  http buf.
*   MD5       :  MD5 form http head(16b).
*           add by alex.lin ---2014-12-04
*************************************************************/
int32 Http_GetMD5( int8 *httpbuf,int8 *MD5)
{
   int8 *p_start = NULL;
   int8 *p_end =NULL;
   int8 Temp_MD5[32]={0};
   int8 MD5_TMP[16];
   int8 Temp[3]={0};
   int8 *str;
   int32 i=0,j=0;
   p_start = strstr( httpbuf,"Firmware-MD5: ");
   if(p_start==NULL)
       return 1;
   p_start = p_start+strlen("Firmware-MD5: ");
   p_end = strstr( p_start,kCRLFNewLine);
   if(p_end==NULL)
       return 1;
   if((p_end-p_start)!=32) return 1;
   memcpy( Temp_MD5,p_start,32);
   
   //memcpy( MD5,p_start,16);
   MD5[16]= '\0';

   for(i=0;i<32;i=i+2)
   {
       Temp[0] = Temp_MD5[i];
       Temp[1] = Temp_MD5[i+1];
       Temp[2] = '\0';
       MD5_TMP[j]= strtol(Temp, &str,16);  
       j++;
   }
   memcpy(MD5,MD5_TMP,16);
   GAgent_Printf(GAGENT_INFO," MD5 From HTTP :----------------");
   
   for(j=0;j<16;j++)
       GAgent_Printf(GAGENT_DUMP,"%02X",MD5[j]);
   
   return 16;
}
/****************************************************************
*       FunctionName     :      Http_Get3rdCloudInfo
*       Description      :      get 3rd cloud name and info form 
*                               buf.
*       buf              :      data afer provision.
*       szCloud3Name     :      3rd cloud name.
*       szCloudInfo      :      3rd cloud info.
*       return           :      1 need 3rd cloud
*                               0 only need gizwits
*       Add by Alex.lin   --2015-03-03
****************************************************************/
uint8 Http_Get3rdCloudInfo( int8 *szCloud3Name,int8 *szCloud3Info,uint8 *buf )
{
    int8 *p_start = NULL;
    int8 *p_end =NULL;
    int8 *cloudName = "3rd_cloud=";
    int8 *uuid = "product_uuid=";
    
    memset( szCloud3Name,0,10 );
    memset( szCloud3Info,0,32 );
    
    p_start = strstr( buf,cloudName );
    if( p_start==NULL )
        return 0;
    p_start+=strlen( cloudName );
    p_end = strstr( p_start,"&");
    if( p_end==NULL )
        return 0;
    memcpy( szCloud3Name,p_start,(p_end-p_start) );
    szCloud3Name[p_end-p_start]='\0';
    
    p_start = strstr( p_start,uuid );
    if( p_start==NULL )
        return 0;
    p_start +=strlen( uuid );
    p_end = strstr( p_start,kCRLFNewLine );
    if( p_end==NULL )
        return 0;
    memcpy( szCloud3Info,p_start,(p_end-p_start));
    szCloud3Info[p_end-p_start]='\0';
    return 1;
}

