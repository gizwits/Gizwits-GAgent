#include "gagent.h"
#define SSID "UPGRADE-AP"
#define KEY  "18018888"
void GAgent_WiFiInit( pgcontext pgc )
{
    int8 ret=0;
    strcpy( pgc->gc.wifi_ssid,SSID );
    strcpy( pgc->gc.wifi_key,KEY );
    ret = GAgent_DRVGetWiFiMode(pgc);
    if( ((pgc->gc.flag)&XPG_CFG_FLAG_CONNECTED) == XPG_CFG_FLAG_CONNECTED )
    {
        GAgent_Printf( GAGENT_INFO,"In Station mode");
        GAgent_Printf( GAGENT_INFO,"SSID:%s,KEY:%s",pgc->gc.wifi_ssid,pgc->gc.wifi_key );
        pgc->rtinfo.GAgentStatus |=WIFI_MODE_STATION;
        pgc->rtinfo.GAgentStatus |= GAgent_DRVWiFi_StationCustomModeStart( pgc->gc.wifi_ssid,pgc->gc.wifi_key,pgc->rtinfo.GAgentStatus );
    }
    else
    {
        GAgent_Printf( GAGENT_INFO,"In AP mode");
        pgc->rtinfo.GAgentStatus |=WIFI_MODE_AP;
        pgc->rtinfo.GAgentStatus |= GAgent_DRV_WiFi_SoftAPModeStart( pgc->minfo.ap_name,AP_PASSWORD,pgc->rtinfo.GAgentStatus );
        
    }
}