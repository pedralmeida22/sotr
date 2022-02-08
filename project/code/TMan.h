#define PRIORITY_TICKS (tskIDLE_PRIORITY + 5)
#define PRIORITY_TASK_A (tskIDLE_PRIORITY + 4)
#define PRIORITY_TASK_B (tskIDLE_PRIORITY + 4)
#define PRIORITY_TASK_C (tskIDLE_PRIORITY + 3)
#define PRIORITY_TASK_D (tskIDLE_PRIORITY + 3)
#define PRIORITY_TASK_E (tskIDLE_PRIORITY + 2)
#define PRIORITY_TASK_F (tskIDLE_PRIORITY + 2)
#define PRINTS_PRIORITY tskIDLE_PRIORITY
#define IMAXCOUNT 10
#define JMAXCOUNT 10
#define PERIOD 200
#define BLOCKED 0
#define RUNNING 1
#define STARTED -1
#define NUMBER_OF_TASKS 7


/*
 definicao da estrutura de uma Task
 */
struct Task {
    const char* name;               // nome task TMan
    int deadline;                   // task deadline em TMan Ticks
    int phase;                      // task phase em TMan Ticks
    int period;                     // task period em TMan Ticks
    const char* precedence;         // nome da task com precedencia
    int currentActivation;          // TMan Tick em que a task foi ativada
    int nextActivation;             // TMan Tick em que a task tem de ser novamente ativada
    int numberOfActivation;         // counter de ativacoes da task
    int deadlineMissedCounter;      // counter de falhas de deadline
    int state;                      // estado da task: started, running ou blocked
    int end;                        // TMan Tick em que a task acabou de executar
};

/*
 * inicializacao de variaveis
 * numero de tasks adicionadas
 * numero de tasks maximo
 * fila de mensagem a imprimir
 */
void TMan_Init(int nMax);

/*
 * terminacao de todas as tasks
 */
void TMan_Close(void);

/*
 * adicionar task ao array de tarefas
 * verificar se pode adicionar
 */
int TMan_TaskAdd(const char* taskName);


/*
 * registar atributos de tarefas periodicas
 */
void TMan_TaskRegisterAttributes(int index, int phase, int period, int deadline);

/*
 * registar atributos de tarefas esporadicas
 * tem precedencias
 */
void TMan_SporadicTaskRegisterAttributes(int index, int deadline, const char* precedence);

/*
 * task espera pela proxima ativacao
 * verificacao de falha de deadline
 */
void TMan_TaskWaitPeriod(void);

/*
 * estatisticas da tasks
 * numero de ativacoes
 * numero de deadline misses
 */
void TMan_TaskStats(const char*);

/*
 * verificar se a task pode executar
 */
void TMan_Ticks(void *pvParams);

/*
 * trabalho de execucao das tasks (consumir tempo)
 */
void Task_Work(void *pvParams);

/*
 * imprimir mensagem em espera na fila
 */
void TMan_Print(void *pvParam);
