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
#include <stdbool.h>

#include "task_mqtt.h"
#include "log.h"

#define BUF_MAX_LEN 8196
#define SERVERIP "127.0.0.1" //my domain or ip
#define SERVERPORT (1883)
#define KEEPALIVE (60)       // 心跳包时长

static int running = 1;
static struct mosquitto *mosq;
static pthread_t loop_thread_id; 
static char msgbuf[BUF_MAX_LEN];
static int tick = 10;
static char *self_topic=NULL;

// 只要代理服务器发送连接应答，就会调用该函数
static void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	if(self_topic!=NULL){
		mosquitto_subscribe(mosq, NULL,self_topic, 1);
	}
	logTrace("\n This is connect callback");
}
// 接收到代理服务器发来的消息时，调用此函数
static void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	// if(!self_topic)
		// return;
    // if(strcmp(message->topic,self_topic)){  // mqtt的topic 是有匹配表达式的，这里的self_topic 指程序订阅的topic
        // return ;
    // }
	
	if(message->payloadlen > BUF_MAX_LEN)
		return ;
    memset(msgbuf,0,BUF_MAX_LEN);
    memcpy(msgbuf,message->payload,message->payloadlen);
    // ANNA_LOG(LOG_DBG,"[receive message]:%s\n",msgbuf);
    printf("[receive message]:%s\n",msgbuf);
    /*解析消息内容，处理后，并反馈结果*/
	//TODO
	logTrace("This is message_callback");
    
}
static void subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	//ANNA_LOG(LOG_DBG,"subscribe callback\n");
	logTrace("This is subscribe_callback");
}
static void log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{   
	logTrace("This is log_callback");
}
static void disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	logTrace("This is disconnect_callback");
}
static void publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	logTrace("This is publish_callback");
}
void  message_publish(char *topic,char *str)
{
	int pub_id=0;
	/*
		libmosq_EXPORT int mosquitto_publish(	struct 	mosquitto 	*	mosq,
		int 	*	mid,
		const 	char 	*	topic,
		int 		payloadlen,
		const 	void 	*	payload,
		int 		qos,
		bool 		retain	)
		
		Publish a message on a given topic.

		Parameters
		mosq		a valid mosquitto instance.
		mid			pointer to an int.  If not NULL, the function will set this to the message id of this particular message.  This can be then used with the publish callback to determine when the message has been sent.  Note that although the MQTT protocol doesn’t use message ids for messages with QoS=0, libmosquitto assigns them message ids so they can be tracked with this parameter.
		topic		null terminated string of the topic to publish to.
		payloadlen	the size of the payload (bytes).  Valid values are between 0 and 268,435,455.
		payload	pointer to the data to send.  If payloadlen > 0 this must be a valid memory location.
		qos			integer value 0, 1 or 2 indicating the Quality of Service to be used for the message.
		retain		set to true to make the message retained.
	*/
	mosquitto_publish(mosq, (int *)&pub_id,topic, strlen(str),str, 1, 0);
}

void  message_subcribe(char *pattern)
{
	int sub_id=0;
	/*
		libmosq_EXPORT int mosquitto_subscribe(	struct 	mosquitto 	*	mosq,
												int 	*	mid,
												const 	char 	*	sub,
												int 		qos	)
		Subscribe to a topic.

		Parameters:
		mosq	a valid mosquitto instance.
		mid		a pointer to an int.  If not NULL, the function will set this to the message id of this particular message.  This can be then used with the subscribe callback to determine when the message has been sent.
		sub		the subscription pattern.
		qos		the requested Quality of Service for this subscription.
	*/
	mosquitto_subscribe(mosq, (int *)&sub_id,pattern, 1);
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
			// ANNA_LOG(LOG_DBG,"reconnect\n");
			logTrace("\n reconnect..%d",rc);
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
	logTrace("\n The uuid is %s..",s_id);
	
	self_topic=(char*)malloc(sizeof(char)*64);
	if(!self_topic){
		// ANNA_LOG(LOG_ERR,"out of memory\n");
		logError("\n out of memory");
		return ret;
	}
	mosquitto_lib_init();        // 库初始化
	mosquitto_threaded_set(mosq,true); // 设置多线程操作模式
	
	/*
	* 	inter to a struct mosquitto on success.  NULL on failure.  Interrogate errno to determine the cause for the failure:
		ENOMEM on out of memory.
		EINVAL on invalid input parameters
	*/
	mosq = mosquitto_new((const char*)s_id, clean_session, NULL); // 创建一个mosquitto 实例
	if(mosq){
		mosquitto_connect_callback_set(mosq,connect_callback);      // 设置mosquitto 连接成功之后调用的函数
		mosquitto_disconnect_callback_set(mosq,disconnect_callback);  // 设置mosquitto 断开连接之后调用的函数
		// mosquitto_log_callback_set(mosq,log_callback);                // 设置日志回调函数
		mosquitto_message_callback_set(mosq,message_callback);        // 设置信息接收回调函数
		mosquitto_subscribe_callback_set(mosq,subscribe_callback);    // 设置订阅的回调函数
		mosquitto_publish_callback_set(mosq,publish_callback);        // 当发布消息成功之后回调的函数
	}else{
		// ANNA_LOG(LOG_ERR,"mosquitto new failed\n");
		logError("\n mosquitto new failed");
		return -1;
	}
	mosquitto_connect(mosq, SERVERIP, SERVERPORT, KEEPALIVE);
	running = 1;
	if(pthread_create(&loop_thread_id, NULL, loopping, 0) != 0){
		// ANNA_LOG(LOG_DBG,"monitor thread create failed\n");
		return -1;
    }

	// ANNA_LOG(LOG_INFO,"mqtt init success!");
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