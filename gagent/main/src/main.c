#include "stdio.h"
#include "gagent.h"
#include "gagent_typedef.h"
extern pgcontext pgContextData;

int main()
{  
    GAgent_Init( &pgContextData );
    GAgent_dumpInfo( pgContextData );
    while(1)
    {
        GAgent_Tick( pgContextData );
        GAgent_SelectFd( pgContextData,1,0 );

        GAgent_Lan_Handle( pgContextData, pgContextData->rtinfo.Rxbuf , GAGENT_BUF_LEN );
        GAgent_Local_Handle( pgContextData, pgContextData->rtinfo.Rxbuf, GAGENT_BUF_LEN );
        GAgent_Cloud_Handle( pgContextData, pgContextData->rtinfo.Rxbuf, GAGENT_BUF_LEN );
    }

}