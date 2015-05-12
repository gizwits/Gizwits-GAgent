#ifndef _LAN_H_
#define _LAN_H_

#include "platform.h"

#define LAN_CLIENT_LOGIN_SUCCESS        0
#define LAN_CLIENT_LOGIN_FAIL           1

#define GAGENT_PROTOCOL_VERSION         (0x00000003)

#define GAGENT_LAN_CMD_ONBOARDING           0X0001
#define GAGENT_LAN_CMD_REPLY_ONBOARDING     0X0002
#define GAGENT_LAN_CMD_ONDISCOVER           0X0003
#define GAGENT_LAN_CMD_REPLY_BROADCAST      0X0004
#define GAGENT_LAN_CMD_STARTUP_BROADCAST    0X0005
#define GAGENT_LAN_CMD_BINDING              0X0006
#define GAGENT_LAN_CMD_LOGIN                0X0008
#define GAGENT_LAN_CMD_TRANSMIT             0X0090
#define GAGENT_LAN_CMD_GETFVRINFO           0X000A
#define GAGENT_LAN_CMD_HOSTPOTS             0X000C
#define GAGENT_LAN_CMD_LOG                  0X0010
#define GAGENT_LAN_CMD_INFO                 0X0013
#define GAGENT_LAN_CMD_TICK                 0X0015
#define GAGENT_LAN_CMD_TEST                 0X0017
#define GAGENT_LAN_REPLY_TEST               0X0018
#define GAGENT_LAN_CMD_AIR_BROADCAST        0X0019

#define LAN_PROTOCOL_HEAD_LEN              4
#define LAN_PROTOCOL_FLAG_LEN              1
#define LAN_PROTOCOL_CMD_LEN               2
#define LAN_PROTOCOL_MCU_ATTR_LEN          8

void Lan_CreateTCPServer(int32 *pFd, uint16 tcp_port);
void Lan_CreateUDPServer(int32 *pFd, uint16 udp_port);
struct sockaddr_t Lan_CreateUDPBroadCastServer(int32 *pFd, uint16 udp_port );
#endif
