
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cy_wcm.h"
#include "cy_wcm_error.h"
#include "queue.h"
#include "networkTask.h"

QueueHandle_t networkQueue;
cy_wcm_ip_address_t ip_addr;

void scanCallback( cy_wcm_scan_result_t *result_ptr, void *user_data, cy_wcm_scan_status_t status )
{
	if(status == CY_WCM_SCAN_COMPLETE)
		return;

	printf("%32s\t%d\t%d\t",result_ptr->SSID,result_ptr->signal_strength,result_ptr->channel);
	switch(result_ptr->band)
	{
		case CY_WCM_WIFI_BAND_ANY:
			printf("ANY");
		break;
		case CY_WCM_WIFI_BAND_5GHZ:
			printf("5.0 GHZ");
		break;
		case CY_WCM_WIFI_BAND_2_4GHZ:
			printf("2.4 GHZ");
		break;
		default:
		printf("%d",result_ptr->channel);
		break;
	}

	printf("\n");
}

void printIp(cy_wcm_ip_address_t *ipad)
{
	if(ip_addr.version == CY_WCM_IP_VER_V4)
			{
				//printf("%d.%d.%d.%d\n",(int)ip_addr.ip.v4>>0&0xFF,(int)ip_addr.ip.v4>>8&0xFF,(int)ip_addr.ip.v4>>16&0xFF,(int)ip_addr.ip.v4>>24&0xFF);
				printf("%d.%d.%d.%d\n",(int)ipad->ip.v4>>0&0xFF,(int)ipad->ip.v4>>8&0xFF,(int)ipad->ip.v4>>16&0xFF,(int)ipad->ip.v4>>24&0xFF);

			}
			else if (ip_addr.version == CY_WCM_IP_VER_V6){
				for(int i=0;i<4;i++)
				{
					printf("%0X:",(unsigned int)ip_addr.ip.v6[i]);
				}
				printf("\n");
			}
			else
			{
				printf("IP ERROR %d\n",ipad->version);
			}			
}

void wcmCallback(cy_wcm_event_t event, cy_wcm_event_data_t *event_data)
{

	cy_wcm_ip_address_t *ipad;
	ipad = (cy_wcm_ip_address_t *)event_data;

	if(ipad == &ip_addr)
	{
		printf("Address are same\n");
	}


	switch(event)
	{
		case CY_WCM_EVENT_CONNECTED:
			printf("Connected\n");
		break;
		case CY_WCM_EVENT_DISCONNECTED:
			printf("Disconnected\n");
		break;
		case    CY_WCM_EVENT_IP_CHANGED:
			printf("IP Address Changed ");
			printIp(ipad);
		break;
	}
}


void networkTask(void *arg)
{
	cy_wcm_config_t config;
	cy_rslt_t result;

	memset(&config, 0, sizeof(cy_wcm_config_t));
    config.interface = CY_WCM_INTERFACE_TYPE_STA;

	cy_wcm_init	(&config);

	cy_wcm_register_event_callback(	wcmCallback	);
	

	networkQueue = xQueueCreate( 5, sizeof(networkQueueMsg_t));

	while(1)
	{
		networkQueueMsg_t msg;
		cy_wcm_connect_params_t connect_params;

		xQueueReceive(networkQueue,(void *)&msg,portMAX_DELAY);
		printf("Received cmd=%d val=%d\n",(int)msg.cmd,(int)msg.val0);
		switch(msg.cmd)
		{
			case net_scan:
				if(msg.val0 == 0)
				{
					cy_wcm_stop_scan();
				}
				else
				{
					cy_wcm_start_scan(scanCallback,0,0);				
				}
				
			break;

			case net_connect:
				
				printf("SSID=%s PW=%s\n",(char *)msg.val0,(char *)msg.val1);
				memset(&connect_params, 0, sizeof(cy_wcm_connect_params_t));
				strcpy((char *)connect_params.ap_credentials.SSID,(char *)msg.val0);
				strcpy((char *)connect_params.ap_credentials.password,(char *)msg.val1);
				free((void *)msg.val0);
				free((void *)msg.val1);
    			connect_params.ap_credentials.security = CY_WCM_SECURITY_WPA2_AES_PSK;
				result = cy_wcm_connect_ap(&connect_params,&ip_addr);
				printf("Connect result=%d\n",(int)result);
			break;
			case net_disconnect:
				cy_wcm_disconnect_ap();
			break;

			case net_printip:
			result = cy_wcm_get_ip_addr	(CY_WCM_INTERFACE_TYPE_STA,&ip_addr,1);
			if(result == CY_RSLT_SUCCESS)
			{
				printf("Ip result=%d ",result);
				printIp(&ip_addr);
			}
			else if(result == CY_RSLT_WCM_NETWORK_DOWN)
				printf("Network disconnected\n");
			else 
				printf("Ip result=%d ",result);
			break;
	
		}
	}

}
