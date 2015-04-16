#include "stdio.h"
#include "gagent.h"
#include "gagent_typedef.h"
#include "http.h"
#include <sys/time.h>

int32 GAgent_SelectFd(pgcontext pgc,int32 sec,int32 usec )
{
    int32 ret=0;
    int32 select_fd=0;
    struct timeval t;
    
    t.tv_sec = sec;// 秒
    t.tv_usec = usec;// 微秒
    
    GAgent_AddSelectFD( pgc );
    select_fd = GAgent_MaxFd( pgc );
    if( select_fd>0 )
    {
        ret = select( select_fd+1,&(pgc->rtinfo.readfd),NULL,NULL,&t );
        if( ret==0 )
        {
            //GAgent_Printf( GAGENT_INFO,"select tiem out!");
        }
    }
    return ret;
}

int main()
{  
    GAgent_Init( &pgContextData );
    GAgent_dumpInfo( pgContextData );
    while(1)
    {
        GAgent_Tick( pgContextData );
        GAgent_SelectFd( pgContextData,1,0 );

        GAgent_Lan_Handle( pgContextData, pgContextData->rtinfo.Rxbuf , pgContextData->rtinfo.Txbuf, GAGENT_BUF_LEN );
        GAgent_Local_Handle( pgContextData, pgContextData->rtinfo.Rxbuf, GAGENT_BUF_LEN );
        GAgent_Cloud_Handle( pgContextData, pgContextData->rtinfo.Rxbuf, GAGENT_BUF_LEN );

    }
    
}