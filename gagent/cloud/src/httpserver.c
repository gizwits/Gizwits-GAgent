int handleWebConfig(int fd)
{
    int read_len;
    char *buf_head, *buf_body, *index_ssid, *index_pass, *p;

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

    if(GAgent_strstr(buf_head, "web_config.cgi") == NULL){
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
        
        fflush(fd);
    }
    else{
        //GET /web_config.cgi?fname=chensf&lname=pinelinda HTTP/1.1
        index_ssid = GAgent_strstr(buf_head, "ssid=");
        index_pass = GAgent_strstr(buf_head, "pass=");
        if(index_ssid && index_pass)
        {
            index_ssid += strlen("ssid=");
            index_pass += strlen("pass=");
            p = strchr(index_ssid, '&');
            if(p) *p = '\0';
            p = strchr(index_pass, ' ');
            if(p) *p = '\0';

            if((strlen(index_ssid) > SSID_LEN_MAX) || (strlen(index_pass) > WIFIKEY_LEN_MAX))
            {
                /* switch to err handle */
            }
            memset(g_stGAgentConfigData.wifi_ssid, 0, SSID_LEN_MAX + 1);
            memset(g_stGAgentConfigData.wifi_key, 0, WIFIKEY_LEN_MAX + 1);

            memcpy(g_stGAgentConfigData.wifi_ssid, index_ssid, strlen(index_ssid));
            memcpy(g_stGAgentConfigData.wifi_key, index_pass, strlen(index_pass));

            g_stGAgentConfigData.flag |= XPG_CFG_FLAG_CONNECTED;
            g_stGAgentConfigData.flag |= XPG_CFG_FLAG_CONFIG;
            DRV_GAgent_SaveConfigData(&g_stGAgentConfigData);

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
            
            fflush(fd);
            
            DRV_WiFi_StationCustomModeStart(g_stGAgentConfigData.wifi_ssid, g_stGAgentConfigData.wifi_key);
            msleep(100);
            DRV_GAgent_Reset();
        }
    }

    free(buf_head);
    free(buf_body);
    msleep(100);
    return 0;
}

/*****************************************************************
 *        Function Name    :    Socket_DoTcpWebConfig
 *        add by chensf     2014-11-06
 *               web config
 *******************************************************************/
void Socket_DoTcpWebConfig(void)
{
    fd_set readfds, exceptfds;
    int ret;
    int newClientfd;
    int i;
    struct sockaddr_t addr;
    struct timeval_t t;

    int addrlen= sizeof(addr);

    if(g_wifiRunningMode != WIFI_MODE_AP)
    {
        if(g_TCPWebConfigFd > 0)
        {
            close(g_TCPWebConfigFd);
            g_TCPWebConfigFd = -1;
        }
        return ;
    }

    if(g_TCPWebConfigFd <= 0)
    {
        Socket_CreateWebConfigServer(80);
        if(g_TCPWebConfigFd <= 0) return ;
    }


    t.tv_sec = 0; /* 秒 */
    t.tv_usec = 0; /* 微秒 */

    FD_ZERO(&readfds);
    FD_SET(g_TCPWebConfigFd, &readfds);

    ret = select(g_TCPWebConfigFd+1, &readfds, NULL, &exceptfds, &t);
    if (ret <= 0)  /* 0 成功 -1 失败 */
    {
        return;
    }

    newClientfd = Socket_accept(g_TCPWebConfigFd, &addr, &addrlen);
    if(newClientfd <= 0) return ;

    handleWebConfig(newClientfd);
    close(newClientfd);
    newClientfd = -1;

    return ;

}
