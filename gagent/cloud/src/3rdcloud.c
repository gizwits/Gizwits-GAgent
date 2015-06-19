#include "gagent.h"
#include "3rdcloud.h"



void GAgent3rdCloud_Handle( pgcontext pgc )
{
    if( 1!= pgc->rtinfo.waninfo.Cloud3Flag )
        return ;
    if( WIFI_CLOUD_CONNECTED!=((pgc->rtinfo.GAgentStatus)&WIFI_CLOUD_CONNECTED) )
        return ;
    
    if( 0==strcmp( pgc->gc.cloud3info.cloud3Name,"jd") )
    {
    }
}
void GAgent3rdCloudTick( pgcontext pgc )
{

}