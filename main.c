#include "task_mqtt.h"

int main()
{
	int ret;
	ret = MqttLocalModuleInit(NULL);
	if(ret != 0)
		return -1;
	while(1){
		sleep(1);
	}
	MqttLocalModuleExit(NULL);
	return 0;
}