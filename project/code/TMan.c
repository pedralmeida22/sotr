/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include <xc.h>

#include "ConfigPerformance.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* App includes */
#include "../UART/uart.h"
#include "semphr.h"

#include "TMan.h"

struct Task tasks[NUMBER_OF_TASKS] = {};
int tasksAdded;
int maxTasks;
TickType_t TMan_Tick;
QueueHandle_t msgs;


void TMan_Init(int nMax){    
    tasksAdded = 0;
    maxTasks = nMax;
    msgs = xQueueCreate(maxTasks * 5,sizeof(char)*80);
    
    xTaskCreate(TMan_Ticks, (const signed char * const ) "ticks", configMINIMAL_STACK_SIZE, NULL, PRIORITY_TICKS, NULL);
    
    xTaskCreate(TMan_Print, ( const signed char * const ) "prints", configMINIMAL_STACK_SIZE, NULL, PRINTS_PRIORITY, NULL );
    
    printf("\n\n---------------------------------------------\n");
    printf("|----------Starting TMAN FRAMEWORK----------|\n");
    printf("---------------------------------------------\n");
}

void TMan_Close(void) {
    
    vTaskEndScheduler();
    int i;
    for(i = 0; i < tasksAdded; i++){
        TaskHandle_t handle = xTaskGetHandle(tasks[i].name);
        vTaskDelete(handle);
    }
    
    tasksAdded = 0;
    
    TaskHandle_t handle = xTaskGetHandle("ticks");
    vTaskDelete(handle);
    
    handle = xTaskGetHandle("prints");
    vTaskDelete(handle);    
}

void TMan_Ticks(void *pvParams) {
    vTaskDelay(PERIOD);
    TickType_t tick = xTaskGetTickCount();
    
    for(;;){
        int i;
        for(i = 0; i < tasksAdded; i++){
            
            TaskHandle_t handle = xTaskGetHandle(tasks[i].name);
            
            if (strcmp(tasks[i].precedence, "x") == 0) {    // nao tem precedencia
                if( (int) TMan_Tick >= tasks[i].nextActivation) { 
                    if ( (int) TMan_Tick <= tasks[i].nextActivation + tasks[i].deadline) { 
                        TaskHandle_t handle = xTaskGetHandle(tasks[i].name);
                        tasks[i].currentActivation = tasks[i].nextActivation;
                        tasks[i].nextActivation += tasks[i].period; 
                        tasks[i].numberOfActivation++;
                        tasks[i].state = RUNNING;
                        vTaskResume(handle);

                    }                
                    else{
                        tasks[i].deadlineMissedCounter++;
                        tasks[i].currentActivation = tasks[i].nextActivation + tasks[i].period;
                        tasks[i].nextActivation += tasks[i].period;
                    }
                }
            }
            
            else{ // tem precedencia
                const char* pre = tasks[i].precedence;
                int indexPre;
                for(int i = 0; i < tasksAdded; i++){
                    if(tasks[i].name == pre) {
                        indexPre = i;
                    }
                }
                
                if((tasks[indexPre].state == BLOCKED) && (tasks[indexPre].end >= tasks[i].currentActivation)) {
                    tasks[i].currentActivation = TMan_Tick;
                    tasks[i].numberOfActivation++;
                    tasks[i].state = RUNNING;
                    vTaskResume(handle);
                }
            }
        }        
        vTaskDelayUntil(&tick, PERIOD);
        TMan_Tick++;
    }
}

void TMan_TaskWaitPeriod(void){
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    
    const char* name = pcTaskGetName(handle);
    // check deadline
    int i;
    for(i = 0; i < tasksAdded; i++) {
        if( strcmp(tasks[i].name, name) == 0) {
            if (tasks[i].currentActivation + tasks[i].deadline < TMan_Tick){
                tasks[i].deadlineMissedCounter++;
            }
        }
    }
    
    // suspend
    vTaskSuspend(NULL);
}

int TMan_TaskAdd(const char* taskName){
    
    if (tasksAdded < maxTasks) {
        int i;
        for(i = 0; i < tasksAdded; i++) {
            if (taskName == tasks[i].name) {
                printf("Task %s already added!\n", taskName);
                return -1;
            }
        }
        
        tasks[tasksAdded].name = taskName;
        printf("Task %s added!\n", tasks[tasksAdded].name);
        tasksAdded++;
        
    }
    else {
        printf("Could not add task %s! Max reached!\n", taskName);
    }
    
}

void Task_Work(void *pvParams) {   
    char mensagem[80];
    for(;;){
        TMan_TaskWaitPeriod();
        
        sprintf(mensagem,"%s, %d \n\r", (const char*) pvParams, xTaskGetTickCount());
        //sprintf(mensagem,"%s, %d \n\r", (const char*) pvParams, TMan_Tick);
        if( xQueueSend(msgs, mensagem, 10) != pdPASS ) { }
        
        int i;
        int j;
        for(i=0; i<IMAXCOUNT; i++){
            for(j=0; j<JMAXCOUNT; j++) {
                int res = 1 + 1;
            }            
        }
        
        for(i = 0; i < tasksAdded; i++){
            if (tasks[i].name == (const char*) pvParams) {
                tasks[i].state = BLOCKED;
                tasks[i].end = TMan_Tick;                
            }
        }
    }
//    OTHER_STUFF (if needed)    
}

void TMan_TaskRegisterAttributes(int index, int phase, int period, int deadline) {
    tasks[index].deadline = deadline;
    tasks[index].phase = phase;
    tasks[index].period = period;
    tasks[index].currentActivation = phase;
    tasks[index].nextActivation = phase;
    tasks[index].numberOfActivation = 0;
    tasks[index].deadlineMissedCounter = 0;
    tasks[index].precedence = "x";
    tasks[index].state = STARTED;
    tasks[index].end = 0;
}

void TMan_SporadicTaskRegisterAttributes(int index, int deadline, const char* precedence) {
    tasks[index].phase = 0;
    tasks[index].period = 0;
    tasks[index].currentActivation = 0;
    tasks[index].nextActivation = 0;
    tasks[index].numberOfActivation = 0;
    tasks[index].deadlineMissedCounter = 0;
    tasks[index].deadline = deadline;
    tasks[index].precedence = precedence;
    tasks[index].state = STARTED;
    tasks[index].end = 0;
}

void TMan_TaskStats(const char* taskName){
    char mensagem[80];
    int i;
    for(i = 0; i < tasksAdded; i++) {
        if (tasks[i].name == taskName) {
            sprintf(mensagem,"Name: %s, nActivatoins: %d, Deadline: \n\r", (const char*) taskName, tasks[i].numberOfActivation, tasks[i].deadlineMissedCounter);
            if( xQueueSend(msgs, mensagem, 10) != pdPASS ) { }
        }
    }
}

void TMan_Print(void *pvParam){
    char mesg[80];
    
    TickType_t xLastWakeTime;
    for(;;){
        if(msgs != NULL){
            for(;;) {            
               if(xQueueReceive(msgs, mesg, portMAX_DELAY) == pdPASS){
                   PrintStr(mesg);
               }
            }        
        }
    }
}

