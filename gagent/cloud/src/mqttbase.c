#include "gagent.h"
#include "mqttbase.h"
#include "mqttlib.h"

int send_packet(int socketid, const void* buf, unsigned int count)
{
    int ret;
    ret = send(socketid, buf, count, 0);
    GAgent_Printf(GAGENT_INFO,"MQTT Send packet length=%d",ret );
    return ret;
}

/*************************************************
 *
 *		Function 				: check mqtt connect
 *		packet_bufferBUF: MQTT receive data
 *		packet_length 	:	the packet length 
 *		return 					:	success 0,error bigger than 0;
 *   add by Ale lin  2014-03-27
 *		
 ***************************************************/
// Use?
int check_mqttconnect( uint8_t *packet_buffer,int packet_length )
{
    if(packet_length < 0)
    {
        GAgent_Printf(GAGENT_INFO,"Error on read packet!");	
        return 1;
    }

    if(packet_buffer[3] != 0x00)
    {
        GAgent_Printf(GAGENT_INFO,"check_mqttconnect CONNACK failed![%d]", packet_buffer[3]);							
        return 3;
    }

    return 0;
}

/*************************************************
 *
 *		Function : check mqtt witch qos 1
 *		packet_bufferBUF: MQTT receive data
 *		packet_length :	the packet length 
 *   add by Ale lin  2014-04-03
 *		
 ***************************************************/
int check_mqttpushqos1( uint8_t *packet_bufferBUF,int packet_length, uint16_t msg_id )
{
    uint16_t msg_id_rcv;
    uint8_t *packet_buffer=NULL;
    packet_buffer = ( uint8_t* )malloc(packet_length);
    memset( packet_buffer,0,packet_length);
    memcpy( packet_buffer,packet_bufferBUF,packet_length);
    if(packet_length < 0)
    {
        GAgent_Printf(GAGENT_INFO,"Error on read packet!");	
        free(packet_buffer);
        return -1;
    }
    if(MQTTParseMessageType(packet_buffer) != MQTT_MSG_PUBACK)
    {
        GAgent_Printf(GAGENT_INFO,"PUBACK expected!");
        free(packet_buffer);
        return -1;						
    }
    msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
    if(msg_id != msg_id_rcv)
    {
        GAgent_Printf(GAGENT_INFO," message id was expected, but message id was found!");
        free(packet_buffer);
        return -1;
    }			
    free(packet_buffer);						
    GAgent_Printf(GAGENT_INFO,"check_mqttpushqos1 OK");
    
    return 1;														
}
int check_mqtt_subscribe( uint8_t *packet_bufferBUF,int packet_length, uint16_t msg_id )
{
    uint16_t msg_id_rcv;
    uint8_t *packet_buffer=NULL;
    packet_buffer = ( uint8_t* )malloc(packet_length);
    if( packet_buffer==NULL )
    {
        free(packet_buffer);
        return -1;
    }
    memset( packet_buffer,0,packet_length);
    memcpy( packet_buffer,packet_bufferBUF,packet_length);
    if(packet_length < 0)
    {
        GAgent_Printf(GAGENT_INFO,"Error on read packet!");
        free(packet_buffer);
        return -1;
    }

    if(MQTTParseMessageType(packet_buffer) != MQTT_MSG_SUBACK)
    {
        GAgent_Printf(GAGENT_INFO,"SUBACK expected!");
        free(packet_buffer);
        return -1;
    }

    msg_id_rcv = mqtt_parse_msg_id(packet_buffer);
    if(msg_id != msg_id_rcv)
    {
        GAgent_Printf(GAGENT_INFO," message id was expected, but message id was found!");
        free(packet_buffer);
        return -1;
    }
    free(packet_buffer);

    return 1;
}

/**********************************************************
 *
 *			Function 	: PubMsg() 
 *			broker	 	: mqtt_broker_handle_t
 *			topic			:	sub topic
 *			Payload		:	msg payload
 *			PayLen		: payload length
 *			flag			: 0 qos 0 1 qos 1
 *			return 		: 0 pub topic success 1 pub topic fail.
 *			Add by Alex lin		2014-04-03
 *
 ***********************************************************/
int PubMsg_( mqtt_broker_handle_t* broker, const char* topic, char* Payload, int PayLen, int flag, void *extra )
{
    uint16_t msg_id;
    int pubFlag=0;

    switch(	flag )
    {
    case 0:	
        pubFlag = XPGmqtt_publish_( broker,topic,Payload,PayLen,0 );
        break;
    case 1:
        pubFlag = XPGmqtt_publish_with_qos_( broker,topic,Payload,PayLen,0,1,&msg_id, extra);
        // <<<<< PUBACK	
        /*
                        packet_length = MQTT_readPacket(g_stSocketRecBuffer,SOCKET_RECBUFFER_LEN);				            
                        if(check_mqttpushqos1( packet_buffer,packet_length,msg_id )) 
                        {
                        pubFlag=0;
                        }
                        else
                        {
                        pubFlag=1;
                        }
        */
        break;
    case 2:
        /* 表示有附加数据，此时qos标志为0xff */
        pubFlag = XPGmqtt_publish_with_qos_( broker,topic,Payload,PayLen,0, 0xFF, &msg_id, extra);
        break;
    default:
        pubFlag=1;
        break;
    }

    return pubFlag;
}
#if 1
int PubMsg( mqtt_broker_handle_t* broker, ppacket pp, int flag, int totallen)
{
    uint16_t msg_id;
    int pubFlag=0;

    switch(flag)
    {
    case 0:
        pubFlag = XPGmqtt_publish(broker, pp, 0);
        break;
    case 1:
        pubFlag = XPGmqtt_publish_with_qos(broker, pp, 0, 1, 0, &msg_id);
        break;
    case 2:
        /* 表示有附加数据，此时qos标志为0xff */
        pubFlag = XPGmqtt_publish_with_qos( broker, pp, totallen, 0, 0xFF, &msg_id);
        break;
    default:
        pubFlag=1;
        break;
    }

    return pubFlag;
}
#endif
