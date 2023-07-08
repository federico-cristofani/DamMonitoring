/* Handle lack of floating point */
#define SCALE_EXP       3
#define MULTIPLIER      1000 /* 10 ^ SCALE_EXP */

/* Flow sensor */
#define MAX_OUTFLOW_1   100 * MULTIPLIER / 100   /* 50 m^3/s */
#define MAX_OUTFLOW_2   35 * MULTIPLIER / 100   /* 35 m^3/s */
#define MAX_INFLOW      80 * MULTIPLIER / 100   /* 80 m^3/s */

/* Gates */
#define INIT_OUTGATE   0  /* 0% (closed) */
#define INIT_INGATE    40 /* No gate on inflow channel, but is usefull to change the value according to the simulation scenario  */

/* Water level sensor*/
#define INIT_WATER_LEVEL    32 * MULTIPLIER         /* 32 m */    
#define MAX_WATER_LEVEL     64 * MULTIPLIER         /* 64 m */
#define INIT_BALANCE_FLOW   (MAX_INFLOW * INIT_INGATE) - (INIT_OUTGATE * MAX_OUTFLOW_1 + INIT_OUTGATE * MAX_OUTFLOW_2)
#define MAX_RESERVOIR       (unsigned long)(1048576 / 16) * MULTIPLIER    /* ~ 1M m^3 */  
#define SURFACE             (unsigned long)(16384 / 16)
#define INIT_RESERVOIR      (unsigned long)(737280 / 16) * MULTIPLIER    

#define MIN_BALANCE_FLOW    (0 - MAX_OUTFLOW_1 - MAX_OUTFLOW_2) * 100
#define MAX_BALANCE_FLOW    (MAX_INFLOW * 100)

/* Sim variations */
#define VAR_THRESHOLD                           0.15 * MULTIPLIER   /* FLOW +- 0.15 m^3/s */
#define STEADY_VAR(target, random, max, min)    target ? (random % (max - min + 1) + min):0;       
#define LINEAR_VAR(random, bound, flow)         random % (((bound > flow) ? (bound - flow):(flow - bound)) / 10) + 100
#define WATER_LEVEL_VAR                         5

#ifdef FLOW_NAME
    #if FLOW_NAME == ingate //Ingate-1
        #define MAX_FLOW MAX_INFLOW
        #define INIT_GATE INIT_INGATE           
    #elif FLOW_NAME == outgate-2 // Outgate-1
        #define MAX_FLOW MAX_OUTFLOW_1
        #define INIT_GATE INIT_OUTGATE
    #elif FLOW_NAME == outgate-1 // Outgate-1
        #define MAX_FLOW MAX_OUTFLOW_2
        #define INIT_GATE INIT_OUTGATE
    #else
        #define MAX_FLOW 0
        #define INIT_GATE 0
    #endif
#else
    #define GATE(i)             ((i == 0) ? "inflow":((i == 1) ? "outflow-1":"outflow-2"))
    #define MAX_FLOW(i)         ((i == 0) ? MAX_INFLOW:((i == 1) ? MAX_OUTFLOW_1:MAX_OUTFLOW_2))
#endif

#define STR_(X) #X
#define STR(X) STR_(X)
