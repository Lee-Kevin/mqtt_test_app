/*******************************************************************************
 * Author			: edwin
 * Email			: 944922198@qq.com
 * Last modified	: 2017-08-43 15:19
 * Filename			: remote_server.c
 * Description		: 
 * *****************************************************************************/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
/*需要安装mosquitto库*/
#include <mosquitto.h>
/*需要安装uuid库*/
#include <uuid/uuid.h>

//域名解析
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "task_mqtt.h"
#include "debug.h"

#define BUF_MAX_LEN 8196
#define SERVERIP "http://www.edwin.com" //my domain or ip
#define SERVERPORT (1883)
#define KEEPALIVE (60)

static int running = 1;
static struct mosquitto *mosq;
static pthread_t loop_thread_id; 
static char msgbuf[BUF_MAX_LEN];
static int tick = 10;
static char *self_topic=NULL;


static void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	if(self_topic!=NULL){
		mosquitto_subscribe(mosq, NULL,self_topic, 1);
	}
}
static void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	if(!self_topic)
		return;
    if(strcmp(message->topic,self_topic)){
        return ;
    }
	if(message->payloadlen > BUF_MAX_LEN)
		return ;
    memset(msgbuf,0,BUF_MAX_LEN);
    memcpy(msgbuf,message->payload,message->payloadlen);
    ANNA_LOG(LOG_DBG,"[receive message]:%s\n",msgbuf);
    
    /*解析消息内容，处理后，并反馈结果*/
	//TODO
    
}
static void subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	//ANNA_LOG(LOG_DBG,"subscribe callback\n");
}
static void log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{   
	//ANNA_LOG(LOG_DBG,"log:%s\n",str);
}
static void disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	//ANNA_LOG(LOG_DBG,"disconnect\n");
}
static void publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	//ANNA_LOG(LOG_DBG,"publish callback\n");
}
static void  message_publish(char *topic,char *str)
{
	int pub_id=0;
	mosquitto_publish(mosq, (int *)&pub_id,self_topic, strlen(str),str, 1, 0);
}

static void* loopping(void *param)
{
	(void*)param;
	int rc = -1;
	while(running){
		rc = mosquitto_loop(mosq, -1, 1);
		if(rc){
			sleep(2);
			rc = mosquitto_reconnect(mosq);
			ANNA_LOG(LOG_DBG,"reconnect\n");
		}
	}
	return NULL;
}

int MqttLocalModuleInit(void *param)
{
	int ret = -1;
	bool clean_session = false;
	uuid_t uuid;
	char s_id[128];
	memset(s_id, 0, 128);
	//generate message id
	uuid_generate(uuid);
	uuid_unparse_upper(uuid, s_id);
	
	self_topic=(char*)malloc(sizeof(char)*64);
	if(!self_topic){
		ANNA_LOG(LOG_ERR,"out of memory\n");
		return ret;
	}
	mosquitto_lib_init();
	mosq = mosquitto_new((const char*)s_id, clean_session, NULL);
	if(mosq){
		mosquitto_connect_callback_set(mosq,connect_callback);
		mosquitto_disconnect_callback_set(mosq,disconnect_callback);
		mosquitto_log_callback_set(mosq,log_callback);
		mosquitto_message_callback_set(mosq,message_callback);
		mosquitto_subscribe_callback_set(mosq,subscribe_callback);
		mosquitto_publish_callback_set(mosq,publish_callback);
	}else{
		ANNA_LOG(LOG_ERR,"mosquitto new failed\n");
		return -1;
	}
	mosquitto_connect(mosq, SERVERIP, SERVERPORT, KEEPALIVE);
	running = 1;
	if(pthread_create(&loop_thread_id, NULL, loopping, 0) != 0){
		ANNA_LOG(LOG_DBG,"monitor thread create failed\n");
		return -1;
    }

	ANNA_LOG(LOG_INFO,"mqtt init success!");
	return 0;
}

int MqttLocalModuleExit(void *param)
{
    running = 0;
    pthread_cancel(loop_thread_id);
    pthread_join(loop_thread_id,NULL);
    mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}
/*
int MqttLocalModuleTick(void *param)
{
    //just testing
	if(tick%5 == 0)
		ANNA_LOG(LOG_DBG,"MqttModuleTick\n");
	if(tick == 0){
		tick = 24*60*60;
	}
	tick--;
	return 0;
}
*/
/*其他模块调用此接口，用于模块间消息传输*/
/*
int Send2MqttLocalModule(int type,void *msg)
{
	ANNA_LOG(LOG_DBG,"Send2MqttModule\n");
	return 0;
}
*/