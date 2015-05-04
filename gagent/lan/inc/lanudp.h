#ifndef _LANUDP_H_
#define _LANUDP_H_

static void Lan_dispatchUdpData(pgcontext pgc, struct sockaddr_t *paddr,
            ppacket prxBuf, ppacket ptxBuf, int32 recLen);
static int32 Lan_udpOnBoarding(pgcontext pgc, u8 *buf);
static void LAN_onDiscoverAck(pgcontext pgc, uint8 *ptxBuf, struct sockaddr_t *paddr);
static void LAN_onBoardingAck(pgcontext pgc, uint8 *ptxBuf, struct sockaddr_t *paddr);

#endif
