
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "networkTask.h"
#include "ntshell.h"
#include "ntlibc.h"
#include "psoc6_ntshell_port.h"

volatile int uxTopUsedPriority ;
TaskHandle_t blinkTaskHandle;
TaskHandle_t networkTaskHandle;

// Global variable with a handle to the shell
ntshell_t ntshell;

void ntShellTask()
{

  printf("Started ntshell\n");
  setvbuf(stdin, NULL, _IONBF, 0);
  ntshell_init(
               &ntshell,
               ntshell_read,
               ntshell_write,
               ntshell_callback,
               (void *)&ntshell);
  ntshell_set_prompt(&ntshell, "AnyCloud> ");
  vtsend_erase_display(&ntshell.vtsend);
  ntshell_execute(&ntshell);
}

// this will start the task

void blinkTask(void *arg)
{
    cyhal_gpio_init(CYBSP_USER_LED,CYHAL_GPIO_DIR_OUTPUT,CYHAL_GPIO_DRIVE_STRONG,0);

    for(;;)
    {
    	cyhal_gpio_toggle(CYBSP_USER_LED);
    	vTaskDelay(500);
    }
}

int main(void)
{
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ; // enable OpenOCD Thread Debugging

    cybsp_init() ;
    __enable_irq();

    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    setvbuf(stdin, NULL, _IONBF, 0);
 
    printf("Started WCM Example\n");

    // Stack size in WORDs
    // Idle task = priority 0
    xTaskCreate(blinkTask, "blinkTask", configMINIMAL_STACK_SIZE,0 /* args */ ,2 /* priority */, &blinkTaskHandle);
    xTaskCreate(networkTask, "networkTask", configMINIMAL_STACK_SIZE*8,0 /* args */ ,4/* priority */, &networkTaskHandle);
    xTaskCreate(ntShellTask, "nt shell task", configMINIMAL_STACK_SIZE*2,0 /* args */ ,1 /* priority */, 0);
    vTaskStartScheduler();


}

/* [] END OF FILE */
