#ifndef _LOCAL_SERVER_H_
#define _LOCAL_SERVER_H_


int MqttLocalModuleInit(void *);
int MqttLocalModuleExit(void *param);
void  message_publish(char *topic,char *str);
void  message_subcribe(char *pattern);
#endif
