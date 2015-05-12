#include "gagent.h"


int32 handleWebConfig( pgcontext pgc,int32 fd)
{
    int32   read_len;
    int8    *buf_head, *buf_body, *index_ssid, *index_pass, *p;
    pgconfig pConfigData=NULL;
    pConfigData = &(pgc->gc);

    if(fd <= 0) return -1;

    buf_head = (char *)malloc(1024);
    if (buf_head == NULL)
    {
        return -1;
    }
    memset(buf_head, 0, 1024);

    read_len = read(fd, buf_head, 1024);
    if(read_len <= 0)
    {
        free(buf_head);
        return -1;
    }

    buf_body = (char *)malloc(1024);
    if (buf_body == NULL)
    {
        free(buf_head);
        return -1;
    }
    
    memset(buf_body, 0, 1024);

    if( strstr(buf_head, "web_config.cgi") == NULL)
    {
        snprintf(buf_body, 1024, "%s", "<html><body><form action=\"web_config.cgi\" method=\"get\">"
                 "<div align=\"center\" style=\"font-size:30px; padding-top:100px;\">"
                 "<p>[0~9],[a~z],[A~Z],[-],[_],[.]</p>"
                 "<p>SSID:<input type=\"text\" name=\"ssid\" style=\"font-size:30px;\"/></p>"
                 "<p>PASS:<input type=\"text\" name=\"pass\" style=\"font-size:30px;\"/></p>"
                 "<p><input type=\"submit\" value=\"OK\" style=\"font-size:30px;\"/></p>"
                 "</form></body></html>");
    
        memset(buf_head, 0, 1024);
        snprintf(buf_head, 1024, "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html;charset=gb2312\r\n"
                 "Content-Length: %d\r\n"
                 "Cache-Control: no-cache\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(buf_body));
        
        send(fd, buf_head, strlen(buf_head), 0);
        send(fd, buf_body, strlen(buf_body), 0);
    }
    else
    {
        //GET /web_config.cgi?fname=chensf&lname=pinelinda HTTP/1.1
        index_ssid = strstr(buf_head, "ssid=");
        index_pass = strstr(buf_head, "pass=");
        if(index_ssid && index_pass)
        {
            index_ssid += strlen("ssid=");
            index_pass += strlen("pass=");
            p = strchr(index_ssid, '&');
            if(p) *p = '\0';
            p = strchr(index_pass, ' ');
            if(p) *p = '\0';

            if((strlen(index_ssid) > 32) || (strlen(index_pass) > 32))
            {
                GAgent_Printf( GAGENT_CRITICAL,"web_config SSID and pass too length !" );
            }
            GAgent_Printf( GAGENT_CRITICAL,"web_config SSID and pass !" );
            memset(pConfigData->wifi_ssid, 0, SSID_LEN_MAX );
            memset(pConfigData->wifi_key, 0, WIFIKEY_LEN_MAX);

            memcpy(pConfigData->wifi_ssid, index_ssid, strlen(index_ssid));
            memcpy(pConfigData->wifi_key, index_pass, strlen(index_pass));
            pConfigData->wifi_ssid[ strlen(index_ssid) ] = '\0';
            pConfigData->wifi_key[ strlen(index_pass) ] = '\0';
            pConfigData->flag |= XPG_CFG_FLAG_CONNECTED;
            pConfigData->flag |= XPG_CFG_FLAG_CONFIG;
            GAgent_DevSaveConfigData( pConfigData );

            snprintf(buf_body, 1024, "%s", "<html><body>"
                     "<div align=\"center\" style=\"font-size:40px; padding-top:100px;\">"
                     "<p>Config the wifi module success</p>"
                     "</body></html>");

            memset(buf_head, 0, 1024);
            snprintf(buf_head, 1024, "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html;charset=gb2312\r\n"
                     "Content-Length: %d\r\n"
                     "Cache-Control: no-cache\r\n"
                     "Connection: close\r\n\r\n",
                     strlen(buf_body));

            send(fd, buf_head, strlen(buf_head), 0);
            send(fd, buf_body, strlen(buf_body), 0);   
            pgc->rtinfo.GAgentStatus &=~ WIFI_MODE_AP;
            pgc->rtinfo.GAgentStatus |= GAgent_DRVWiFi_StationCustomModeStart( pgc->gc.wifi_ssid,pgc->gc.wifi_key,pgc->rtinfo.GAgentStatus );
            msleep(100);
        }
    }
    free(buf_head);
    free(buf_body);
    msleep(100);
    return 0;
}
/****************************************************************
Function    :   Socket_CreateWebConfigServer
Description :   creat web config server.
tcp_port    :   web server port.#default 80
return      :   the web socket id .
Add by Alex.lin     --2015-04-24.
****************************************************************/
int32 GAgent_CreateWebConfigServer( uint16 tcp_port )
{   
    return GAgent_CreateTcpServer( tcp_port );
}
/****************************************************************
Function    :   GAgent_DoTcpWebConfig
Description :   creat web config server.
return      :   NULL
Add by Alex.lin     --2015-04-25.
****************************************************************/
void GAgent_DoTcpWebConfig( pgcontext pgc )
{
    uint16 GAgentStatus=0,i=0;
    int32 newfd;
    struct sockaddr_t addr;
    int32 addrLen= sizeof(addr);
    GAgentStatus = pgc->rtinfo.GAgentStatus;

    if( (GAgentStatus&WIFI_MODE_AP)!= WIFI_MODE_AP )
    {
        if( pgc->ls.tcpWebConfigFd>0 )
        {
            close( pgc->ls.tcpWebConfigFd );
            pgc->ls.tcpWebConfigFd = INVALID_SOCKET;
        }
        return ;
    }

    if( pgc->ls.tcpWebConfigFd <= 0 )
    {
        GAgent_Printf( GAGENT_DEBUG,"Creat Tcp Web Server." );
        pgc->ls.tcpWebConfigFd = GAgent_CreateWebConfigServer( 80 );   
    }

    if(FD_ISSET(pgc->ls.tcpWebConfigFd, &(pgc->rtinfo.readfd)))
    {
        /* if nonblock, can be done in accept progress */
        newfd = Socket_accept(pgc->ls.tcpWebConfigFd, &addr, &addrLen);
        if(newfd > 0)
        {
            Lan_AddTcpNewClient(pgc, newfd, &addr);
        }
    }

    for(i = 0;i < LAN_TCPCLIENT_MAX; i++)
    {
        int32 fd=0;
        fd = pgc->ls.tcpClient[i].fd;
        if(fd <= 0)
            continue;
        if(FD_ISSET(fd, &(pgc->rtinfo.readfd)))
        {
            Lan_setClientTimeOut(pgc, i);
            handleWebConfig( pgc,fd);
            close(fd);
            fd = INVALID_SOCKET;
            pgc->ls.tcpClient[i].fd=INVALID_SOCKET;
        }
    }
}



