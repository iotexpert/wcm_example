
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
cy_wcm_mac_t mac_addr;


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

void printMac(cy_wcm_mac_t mac)
{
	for(int i=0;i<CY_WCM_MAC_ADDR_LEN;i++)
	{
		uint8_t val = mac[i];
		printf("%02X:",val);

	}
}

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

	printf("\t");
	printf("%d",(int)result_ptr->max_data_rate);
	printf("\t");
	switch(result_ptr->bss_type)
	{
		case CY_WCM_BSS_TYPE_INFRASTRUCTURE:
			printf("INFR");
		break; 	
		case CY_WCM_BSS_TYPE_ADHOC: 
			printf("ADHOC");
		break;	
		case CY_WCM__BSS_TYPE_ANY: 	
			printf("ANY");
		break;
		case CY_WCM_BSS_TYPE_MESH:
			printf("MESG");
		break;
		case CY_WCM_BSS_TYPE_UNKNOWN: 
			printf("UNKWN");
		break;
	}
	printf("\t");
	printf("%c%c",result_ptr->ccode[0],result_ptr->ccode[1]);
	printf("\t");
	printMac(result_ptr->BSSID);

	printf("\t");
	switch(result_ptr->security)
	{
		case CY_WCM_SECURITY_OPEN:
			printf("OPEN");
		break;
    	case CY_WCM_SECURITY_WEP_PSK:
			printf("WEP_PSK");
		break;
		case CY_WCM_SECURITY_WEP_SHARED:
			printf("WEP_SHARED");
		break;
    	case CY_WCM_SECURITY_WPA_TKIP_PSK:
			printf("WPA_TKIP_PSK");
		break;
		case CY_WCM_SECURITY_WPA_AES_PSK:
			printf("WPA_AES_PSK");
		break;
		case CY_WCM_SECURITY_WPA_MIXED_PSK:
			printf("WPA_MIXED_PSK");
		break;
		case CY_WCM_SECURITY_WPA2_AES_PSK:
			printf("WPA2_AES_PSK");
		break;
		case CY_WCM_SECURITY_WPA2_TKIP_PSK:
			printf("WPA2_TKIP_PSK");
		break;
		case CY_WCM_SECURITY_WPA2_MIXED_PSK:
			printf("WPA2_MIXED_PSK");
		break;
		case CY_WCM_SECURITY_WPA2_FBT_PSK:
			printf("WPA2_FBT_PSK");
		break;
		case CY_WCM_SECURITY_WPA3_SAE:
			printf("WPA3_SAE");
		break;
		case CY_WCM_SECURITY_WPA3_WPA2_PSK:
			printf("WPA3_WPA2_PSK");
		break;
		case CY_WCM_SECURITY_IBSS_OPEN:
			printf("IBSS_OPEN");
		break;
		case CY_WCM_SECURITY_WPS_SECURE:
			printf("WPS_SECURE");
		break;
		case CY_WCM_SECURITY_UNKNOWN:
			printf("UNKNOWN");
		break;

		case CY_WCM_SECURITY_FORCE_32_BIT:
			printf("FORCE_32_BIT");
		break;
	}

	printf("\n");
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
				printf("Ip result=%d ",(int)result);
				printIp(&ip_addr);
			}
			else if(result == CY_RSLT_WCM_NETWORK_DOWN)
				printf("Network disconnected\n");
			else 
				printf("Ip result=%d ",(int)result);
			break;

			case net_printmac:
				result = cy_wcm_get_mac_addr(CY_WCM_INTERFACE_TYPE_STA,&mac_addr,1);
				printf("res=%d MAC Address =",(int)result);
				printMac(mac_addr);
				printf("\n");
			break;
	
		}
	}

}
