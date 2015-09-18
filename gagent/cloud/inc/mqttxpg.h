#ifndef __MQTTLIB_H__
#define __MQTTLIB_H__

#ifndef WIN32
#include <stdint.h>
#endif
#include "gagent.h"
#include "mqttlib.h"
#include "mqttbase.h"

int32 MQTT_readPacket( int32 socketid,ppacket pbuf, int32 bufferLen);
int MQTTclose_socket(mqtt_broker_handle_t* broker);


int32 Mqtt_DoSubTopic( pgcontext pgc,int16 mqttstatus );
int32 Mqtt_SubLoginTopic( mqtt_broker_handle_t *LoginBroker,pgcontext pgc,int16 mqttstatus );
int32 Mqtt_SendConnetPacket( mqtt_broker_handle_t *pstBroketHandle, int32 socketid,const int8* username,const int8* password );
int32 Mqtt_Login2Server(  int32 socketid,const uint8 *username,const uint8 *password );
int32 Mqtt_DoLogin( mqtt_broker_handle_t *LOG_Sendbroker,u8* packet,int32 packetLen );
int32 Mqtt_DispatchPublishPacket( pgcontext pgc,u8 *packetBuffer,int32 packetLen );
int32 Mqtt_IntoRunning( pgcontext pgc );
int32 WAN_DoMCUCommand(u8 clientid[32], u8 *pP0Data, int32 pP0Datalen);

void MQTT_HeartbeatTime(void);
void Mqtt_ReqOnlineClient(pgcontext pgc);
int32 MQTT_SenData( pgcontext pgc, int8 *szDID, ppacket pbuf,int32 buflen );


#endif  /* __LIBEMQTT_H__ */
