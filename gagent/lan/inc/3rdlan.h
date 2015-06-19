#ifndef _3RDLAN_H_
#define _3RDLAN_H_

#define APP2WIFI_BROADCAST              1
#define APP2WIFI_BROADCAST_ACK          2
#define APP2WIFI_WRITE                  3
#define APP2WIFI_WRITE_ACK              4
#define JD2HEADER_LEN                   19

int32 Socket_CreateUDPServer_JD(int32 udp_port);
void GAgent3rdLan_Handle( pgcontext pgc );
void Lan3rdCloudUDPHandle_JD( pgcontext pgc,struct sockaddr_t *paddr,
                                    ppacket prxBuf, int32 recLen );

#endif