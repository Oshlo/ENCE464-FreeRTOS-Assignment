#ifndef __QUAD_TEST_H__
#define __QUAD_TEST_H__


#define TASK_STACK_DEPTH    32
#define TASK_PRIORITY       4

#define QUEUE_LENGTH 5
#define QUAD_QUEUE_ITEM_SIZE sizeof(int8_t)


//void initClock (void);
void quadIntHandler(void);

void initGPIO(void);

void getQuadData (void *pvParameters);

uint8_t initQuadTest(void);

#endif

