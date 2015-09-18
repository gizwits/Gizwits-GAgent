#ifndef __MQTTPUB_H__
#define __MQTTPUB_H__
#include "gagent_typedef.h"
#include "mqttlib.h"

#define RCVBUFSIZE 1024

int send_packet(int socket_info, const void* buf, unsigned int count);

int Cloud_MQTT_initSocket( mqtt_broker_handle_t* broker,int flag );
int read_Mqtt_Ackpacket(int timeout,uint8_t *packet_buffer,int Type,int flage);
int read_Pub_packet(int timeout,uint8_t *packet_buffer);
int read_Sub_packet(int timeout,uint8_t *packet_buffer,int flage);
int check_mqttconnect( uint8_t *packet_bufferBUF,int packet_length );
int check_mqttpushqos1( uint8_t *packet_bufferBUF,int packet_length, uint16_t msg_id );
int check_mqtt_subscribe( uint8_t *packet_bufferBUF,int packet_length, uint16_t msg_id );
int PubMsg_( mqtt_broker_handle_t* broker, const char* topic, char* Payload, int PayLen, int flag, void* extra);
int PubMsg( mqtt_broker_handle_t* broker, ppacket pp, int flag, int totallen);
#endif
