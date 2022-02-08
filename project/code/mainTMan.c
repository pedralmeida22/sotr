/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* App includes */
#include "../UART/uart.h"

#include "TMan.h"
#include "semphr.h"


void configUart(){
    // Init UART and redirect stdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

     __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
    // Disable JTAG interface as it uses a few ADC ports
    DDPCONbits.JTAGEN = 0;
    
    // Initialize ADC module
    // Polling mode, AN0 as input
    // Generic part
    AD1CON1bits.SSRC = 7; // Internal counter ends sampling and starts conversion
    AD1CON1bits.CLRASAM = 1; //Stop conversion when 1st A/D converter interrupt is generated and clears ASAM bit automatically
    AD1CON1bits.FORM = 0; // Integer 16 bit output format
    AD1CON2bits.VCFG = 0; // VR+=AVdd; VR-=AVss
    AD1CON2bits.SMPI = 0; // Number (+1) of consecutive conversions, stored in ADC1BUF0...ADCBUF{SMPI}
    AD1CON3bits.ADRC = 1; // ADC uses internal RC clock
    AD1CON3bits.SAMC = 16; // Sample time is 16TAD ( TAD = 100ns)
    // Set AN0 as input
    AD1CHSbits.CH0SA = 0; // Select AN0 as input for A/D converter
    TRISBbits.TRISB0 = 1; // Set AN0 to input mode
    AD1PCFGbits.PCFG0 = 0; // Set AN0 to analog mode
    // Enable module
    AD1CON1bits.ON = 1; // Enable A/D module (This must be the **last instruction of configuration phase**)
}

/*
 * Create the demo tasks then start the scheduler.
 */
int mainTMan( void *pvParam)
{    
    
    configUart();
    
    TMan_Init(NUMBER_OF_TASKS);
    
    char *tasks_name[NUMBER_OF_TASKS] = {"A", "B", "C", "D", "E", "F"};
    
    xTaskCreate(Task_Work, (const signed char * const ) tasks_name[0], configMINIMAL_STACK_SIZE, (void *) tasks_name[0], PRIORITY_TASK_A, NULL);
    xTaskCreate(Task_Work, (const signed char * const ) tasks_name[1], configMINIMAL_STACK_SIZE, (void *) tasks_name[1], PRIORITY_TASK_B, NULL);
    xTaskCreate(Task_Work, (const signed char * const ) tasks_name[2], configMINIMAL_STACK_SIZE, (void *) tasks_name[2], PRIORITY_TASK_C, NULL);
    xTaskCreate(Task_Work, (const signed char * const ) tasks_name[3], configMINIMAL_STACK_SIZE, (void *) tasks_name[3], PRIORITY_TASK_D, NULL);
    xTaskCreate(Task_Work, (const signed char * const ) tasks_name[4], configMINIMAL_STACK_SIZE, (void *) tasks_name[4], PRIORITY_TASK_E, NULL);
    xTaskCreate(Task_Work, (const signed char * const ) tasks_name[5], configMINIMAL_STACK_SIZE, (void *) tasks_name[5], PRIORITY_TASK_F, NULL);
    
    TMan_TaskAdd(tasks_name[0]);
    TMan_TaskAdd(tasks_name[1]);
    TMan_TaskAdd(tasks_name[2]);
    TMan_TaskAdd(tasks_name[3]);
    TMan_TaskAdd(tasks_name[4]);
    TMan_TaskAdd(tasks_name[5]);
    
    // int index, int phase, int period, int deadline
    TMan_TaskRegisterAttributes(0, 0, 1, 2);
    TMan_SporadicTaskRegisterAttributes(1, 2, tasks_name[5]);
    TMan_TaskRegisterAttributes(2, 0, 3, 2);
    TMan_TaskRegisterAttributes(3, 1, 3, 2);
    TMan_TaskRegisterAttributes(4, 0, 4, 2);
    TMan_TaskRegisterAttributes(5, 2, 4, 2);
  
    
    vTaskStartScheduler();
    
//    TMan_TaskStats(tasks_name[0]);
//    TMan_TaskStats(tasks_name[1]);
//    TMan_TaskStats(tasks_name[2]);
//    TMan_TaskStats(tasks_name[3]);
//    TMan_TaskStats(tasks_name[4]);
//    TMan_TaskStats(tasks_name[5]);
    
    TMan_Close();
            
	return 0;
}
