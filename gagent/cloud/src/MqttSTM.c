#include "gagent.h"
//#include "lib.h"
#include "MqttSTM.h"
#include "mqttlib.h"
//#include "wifi.h"

//int g_MqttCloudSocketID = -1;

int send_packet(int socketid, const void* buf, unsigned int count)
{
    int ret;
    ret = send(socketid, buf, count, 0);
    GAgent_Printf(GAGENT_INFO,"MQTT Send packet length=%d",ret );
    return ret;
}
/******************************************************************
 *       function    :   Cloud_MQTT_initSocket
 *       broker      :   mqtt struct
 *       flag        :   0 wifi module to register socketid.
 *                       1 wifi module to login socketid
 *
 *
 ********************************************************************/
//int Cloud_MQTT_initSocket( mqtt_broker_handle_t* broker,int flag )
//{
//    int ret;
//    int MTflag;
//    struct sockaddr_t Msocket_address;
//    char MqttServerIpAddr[32];
//    int iSocketId;
//    int packetLen;

//#ifdef GAGENT_V4PROTOCOL
//    char *domain = g_Xpg_GlobalVar.m2m_SERVER;
//    unsigned short port = g_Xpg_GlobalVar.m2m_Port;
//#endif
//    memset(MqttServerIpAddr, 0x0, sizeof(MqttServerIpAddr));

//    if(strlen(g_stGAgentConfigData.m2m_ip) < 1)
//    {
//        GAgent_Printf(GAGENT_INFO,"g_stGAgentConfigData.m2m_ip is NULL");
//        return -1;
//    }
//    GAgent_Printf(GAGENT_INFO,"connect to m2m, ip : %s",g_stGAgentConfigData.m2m_ip);
//    strcpy(MqttServerIpAddr, g_stGAgentConfigData.m2m_ip);
//#ifdef GAGENT_WITH_HF
//#ifdef GAGENT_V3PROTOCOL
//    GAgent_DOFirstWifiup();
//#endif
//#endif
//    GAgent_Printf(GAGENT_INFO,"MQTT Connect Domain:%s IP:%s \r\n", domain, MqttServerIpAddr);

//    if( g_MqttCloudSocketID!=-1 )
//    {
//        GAgent_Printf(GAGENT_INFO,"Cloud_MQTT_initSocket close[%04x] mqtt socket", g_MqttCloudSocketID);
//        close_socket(broker);
//    }

//    if( (iSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
//    {
//        GAgent_Printf(GAGENT_ERROR," MQTT socket init fail");
//        return 1;
//    }
//#if(GAGENT_WITH_HF == 1)
//    set_socknoblk(iSocketId);
//#endif
//    ret = connect_mqtt_socket(iSocketId, &Msocket_address, port, MqttServerIpAddr);
//    if (ret < 0)
//    {
//        GAgent_Printf(GAGENT_WARNING,"mqtt socket connect fail with:%d", ret);
//        close(iSocketId);
//        return 3;
//    }
//    /* For Test MQTT Read And Write Func */
//    //MQTT_handlePacket();

//    GAgent_Printf(GAGENT_INFO, "Created mqtt socketid[%08x]", iSocketId);
//    g_MqttCloudSocketID = iSocketId;

//    /* MQTT stuffs */
//    mqtt_set_alive(broker, MQTT_SET_ALIVE);
//    broker->socketid = iSocketId;
//    broker->mqttsend = send_packet;

//    return 0;
//}

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
int PubMsg( mqtt_broker_handle_t* broker, const char* topic, char* Payload, int PayLen, int flag )
{
    uint16_t msg_id;
    int pubFlag=0;

    switch(	flag )
    {
    case 0:	
        pubFlag = XPGmqtt_publish( broker,topic,Payload,PayLen,0 );
        break;
    case 1:	
        pubFlag = XPGmqtt_publish_with_qos( broker,topic,Payload,PayLen,0,1,&msg_id);
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
    default:
        pubFlag=1;
        break;
    }

    return pubFlag;
}
