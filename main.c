#include "task_mqtt.h"
#include <stddef.h>
#include "log.h"

int main()
{
	logInit("loginfo.log");         // 初始化 log脚本  LOGFLAG_SYSLOG
	#if 0
	logSetFlags(LOGFLAG_FILE |
				LOGFLAG_ERROR);
	#else
	logSetFlags(LOGFLAG_FILE |
				LOGFLAG_INFO |
				LOGFLAG_ERROR |
				LOGFLAG_WARN |
				LOGFLAG_TRACE|
				LOGFLAG_DEBUG);
	#endif
	logDebug("Begin the Application");
	
	int ret;
	ret = MqttLocalModuleInit(NULL);
	if(ret != 0)
		return -1;
	while(1){
		char chartemp[10];
		printf("> Please input the cmd:\n>");
	    scanf("%s",chartemp);
		if(strcmp("pub",chartemp) == 0) {
			printf("\n please input the topic\n");
			char topic[10];
			scanf("%s",topic);
			printf("\n please input the message\n");
			char msg[10];
			scanf("%s",msg);
			message_publish(topic,msg);
			printf("\n message_publish done\n");
		} else if (strcmp("sub",chartemp) == 0) {
			printf("\n please input the sub pattern\n");
			char pattern[10];
			scanf("%s",pattern);
			message_subcribe(pattern);
			printf("\n message_subcribe done\n");
		} else {
			printf("\n Please input again! \n");
		}
	}
	MqttLocalModuleExit(NULL);
	return 0;
}