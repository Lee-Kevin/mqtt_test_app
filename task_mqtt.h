#ifndef _LOCAL_SERVER_H_
#define _LOCAL_SERVER_H_

int MqttLocalModuleInit(void *);
//int MqttLocalModuleExit(void *);
//int MqttLocalModuleTick(void *);
int Send2MqttLocalModule(int type,void *msg);

#endif
