/*
 /*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief This is the application entry point.
 *          FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 *          @see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include <stdio.h>
#include "LPC17xx.h"
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "printf_lib.h"
//these are for I2C lab
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "i2c2.hpp"
#include "io.hpp"
#include "Storage.hpp"
#include "event_groups.h"
#include "command_handler.hpp"


/**
 * The main() creates tasks or "threads".  See the documentation of scheduler_task class at scheduler_task.hpp
 * for details.  There is a very simple example towards the beginning of this class's declaration.
 *
 * @warning SPI #1 bus usage notes (interfaced to SD & Flash):
 *      - You can read/write files from multiple tasks because it automatically goes through SPI semaphore.
 *      - If you are going to use the SPI Bus in a FreeRTOS task, you need to use the API at L4_IO/fat/spi_sem.h
 *
 * @warning SPI #0 usage notes (Nordic wireless)
 *      - This bus is more tricky to use because if FreeRTOS is not running, the RIT interrupt may use the bus.
 *      - If FreeRTOS is running, then wireless task may use it.
 *        In either case, you should avoid using this bus or interfacing to external components because
 *        there is no semaphore configured for this bus and it should be used exclusively by nordic wireless.
 */
class GPIOTask : public scheduler_task
{
    public:
        GPIOTask(uint8_t priority) :
            scheduler_task("SSP", 512*4, priority)
        {

        }
         bool init(void)
        {
            //LPC_PINCON->PINSEL2 &= ~0x3;
            //LPC_PINCON->PINSEL2 &= ~((1<<2)|(1<<3));
            LPC_PINCON->PINSEL2 &= ~((1<<8)|(1<<9));
            LPC_PINCON->PINSEL2 &= ~((1<<20)|(1<<21));
            //LPC_PINCON->PINSEL2 &=
            //LPC_PINCON->PINSEL2 &= ~(0x3<<18);
            //BIT(LPC_PINCON->PINSEL2).b19_18 = 0;

            //DIR = 0 is input DIR = 1 is output
            //LPC_GPIO1->FIODIR |= (1<<0);
           // LPC_GPIO1->FIODIR |= (1<<1);
            LPC_GPIO1->FIODIR |= (1<<4);
            LPC_GPIO1->FIODIR &= ~(1<<10);
        //    LPC_GPIO2->FIODIR != (1<<6);
            return true;
        }

        bool run(void *p)
        {
            //Each bit corresponds to 1 physical pin
            //LPC_GPIO2-> FIOPIN & (0 << 6);
            LPC_GPIO2-> FIODIR |= (1 << 6);
            LPC_GPIO2-> FIOPIN |= (1 << 6);
            if (LPC_GPIO1->FIOPIN & (1 << 10))
            {
                BIT(LPC_GPIO1->FIOPIN).b0 = 0;
                BIT(LPC_GPIO1->FIOPIN).b1 = 0;
                BIT(LPC_GPIO1->FIOPIN).b4 = 0;
                // HIGH
            }
            else
            {
                BIT(LPC_GPIO1->FIOPIN).b0 = 1;
                BIT(LPC_GPIO1->FIOPIN).b1 = 1;
                BIT(LPC_GPIO1->FIOPIN).b4 = 1;
                //LOW
            }
            //vTaskDelay(100);
            //BIT(LPC_GPIO1->FIOPIN).b1 = 1;
            //vTaskDelay(100);
            //BIT(LPC_GPIO1->FIOPIN).b1 = 0;
            //BIT(LPC_GPIO1->FIOPIN).b9 = 0;
            return true;
        }
};
class SSPTask : public scheduler_task  //SSP LAB 2
{
    public:
        SSPTask(uint8_t priority) :
            scheduler_task("SSP", 512*4, priority)
        {
            /* Nothing to init */
        }
        bool init(void)
        {
            LPC_PINCON->PINSEL0 &= ~(3<<12);
            LPC_PINCON->PINSEL0 &= ~(3<<14);
            LPC_PINCON->PINSEL0 |= (2<<14);
            BIT(LPC_PINCON->PINSEL0).b19_18 = 2;
            BIT(LPC_PINCON->PINSEL0).b17_16 = 2;
            LPC_SSP1->CR0 = 7;
            LPC_SSP1->CR1 = 2;
            LPC_SSP1->CPSR = 2;

        }
        uint8_t ssptix(uint8_t in)
        {
            LPC_SSP1->DR=in;
            while(LPC_SSP1->SR &= (1<<4));
            return LPC_SSP1->DR;
        }

        bool run(void *p)
        {
            LPC_GPIO0->FIOCLR=(1<<6);
           // vTaskDelay(2500);
            ssptix(0x9f);
            //LPC_GPIO0->FIOSET=(1<<6);
            vTaskDelay(2500);
            printf("\n****************************************************\n");
            printf("\n Line 1: Manufacturer ID\t %x",ssptix(0x00));
            printf("\n Line 2: Device ID part 1\t %x",ssptix(0x00));
            printf("\n Line 3: Device ID part 2\t %x",ssptix(0x00));
            printf("\n Line 4: EDI String Length \t %x",ssptix(0x00));
            printf("\n Line 5: EDI data byte \t %x",ssptix(0x00));
            printf("\n****************************************************\n");
            LPC_GPIO0->FIOSET=(1<<6);
           // vTaskDelay(2500);
            LPC_GPIO0->FIOCLR=(1<<6);
            ssptix(0xD7);
            //LPC_GPIO0->FIOSET=(1<<6);
            printf("\n****************************************************\n");
            printf("\n Line 6: Status Register read part 1\t %x",ssptix(0x00));
            printf("\n Line 7: Status Register read part 2\t %x",ssptix(0x00));
            printf("\n****************************************************\n");
            LPC_GPIO0->FIOSET=(1<<6);
            vTaskDelay(2500);
            //We are done here for regular assignment
            return true;
        }
};
class UARTTask : public scheduler_task  //SSP LAB 2
{
    public:
        UARTTask(uint8_t priority) :
            scheduler_task("UART", 512*4, priority)
        {
            /* Nothing to init */
        }
        bool init(void)
        {
            uint32_t baud =38400;
            uint8_t dll;
            //powering up UART2
            //LPC_SC->PCONP |= (1 <<24);

            //powering up UART3
            LPC_SC->PCONP |= (1 << 25);
            //BIT(LPC_SC->PCLKSEL1).b17_16 = 2;

            //PCLKSEL for UART2
            //LPC_SC->PCLKSEL1 &= ~(3 << 16);
            //LPC_SC->PCLKSEL1 |= (1 << 16);

            //PCLKSEL for UART3
            LPC_SC->PCLKSEL1 &= ~(3 << 18);
            LPC_SC->PCLKSEL1 |= (1 << 18);

            //TXD3
            LPC_PINCON->PINSEL0 &= ~(3 << 0);
            LPC_PINCON->PINSEL0 |= (2 << 0);

            //RXD3
            LPC_PINCON->PINSEL0 &= ~(3 << 2);
            LPC_PINCON->PINSEL0 |= (2 << 2);

            //dll value

            dll = sys_get_cpu_clock()/(16 * baud);

//            LPC_UART2->LCR = (1<<7); #not used
//            LPC_UART2->DLL = dll; #not used

            LPC_UART3->LCR = (1<<7);
            LPC_UART3->DLL = dll;

//            BIT(LPC_PINCON->PINSEL0).b1_0 = 2;   #not used
//            BIT(LPC_PINCON->PINSEL0).b3_2 = 2;   #not used


//            LPC_UART2->LCR = 0x83;
            LPC_UART3->DLM = 0;

            //LPC_UART2->LCR = 0x03;
            //LPC_UART3->LCR = 0x03;
            LPC_UART3->LCR = 0x03;

            //LPC_UART2->FCR = 0x07;
            //LPC_UART3->FCR = 0x07;

        }
        char uart2_putchar(char out)
        {
            LPC_UART3->THR = out;
            while(!(LPC_UART3->LSR & (1 << 6)));
            return 1;
        }
        char uart2_getchar(void)
        {
            while(LPC_UART3->LSR & (1 << 0))
            {
                break;
            }
            char c = LPC_UART3->RBR;
            return c;
        }
        bool run(void *p)
        {
            char a = 'F';
            char b;
            printf("Receiving!\n");
            uart2_putchar(a);

            b=uart2_getchar();
            vTaskDelay(1000);
            printf("The b char is: %c \n",b);
            //We are done here for regular assignment
            return true;
        }
};


class INTTask : public scheduler_task  //SSP LAB 2
{
    public:
       INTTask(uint8_t priority) :
            scheduler_task("INT", 512*4, priority)
        {
            /* Nothing to init */
        }
        static void EINT3_ISR(void)
        {
            if((LPC_GPIOINT->IO2IntStatR|=(1 << 6)) == (1 << 6))
            {
                u0_dbg_printf("Hi, my name is nick.\n");
                LPC_GPIOINT->IO2IntClr |=(1 << 6); //table 123,
                return;
            }
            else if((LPC_GPIOINT->IO2IntStatR |= (1 << 7)) == (1 << 7))
            {
              u0_dbg_printf("Hi, my name is nick 2nd.\n");
              LPC_GPIOINT->IO2IntClr |=(1 << 7); //table 123,
              return;
            }

        }
        bool init(void)
        {
           LPC_PINCON->PINSEL4 &= (0 << 12);   //GPIO 2.6
           LPC_PINCON->PINSEL4 &= (0 << 14);   //GPIO 2.7

           LPC_GPIO2->FIODIR |= (0 << 6);
           LPC_GPIO2->FIODIR |= (0 << 7);

           //BIT(LPC_GPIOINT->IO2IntEnR).b7_6 = 1;
           LPC_GPIOINT->IO2IntEnR |=(1 << 6);   //LPC17xx.h, line 279, GPIO 1
           LPC_GPIOINT->IO2IntEnR |=(1 << 7);   //LPC17xx.h, line 279, GPIO 2

           NVIC_EnableIRQ(EINT3_IRQn);   //vendor supplied, no complaints yet
           //NVIC_EnableIRQ(EINT2_IRQn);

           isr_register(EINT3_IRQn,EINT3_ISR);   //startup.cpp line 338
           //isr_register(EINT3_IRQn,EINT3_ISR2);

           return true;
        }
        bool run(void *p)
        {

            return true;
        }
};

class I2CTask : public scheduler_task
{
    public:
        I2CTask(uint8_t priority) :
            scheduler_task("I2C", 512*4, priority)
        {

        }
         bool init(void)
        {

            i2c.SlaveInit(slaveADDR,buffer,sizeof(buffer));
            return true;
        }
        bool run(void *p)
        {
            return true;
        }

    private:
        uint8_t slaveADDR = 0x10;
        I2C2& i2c = I2C2::getInstance();
        uint8_t buffer[256] = {0};

};

/// IDs used for getSharedObject() and addSharedObject()
typedef enum {
   shared_SensorQueueId,
} sharedHandleId_t;

// Orientation type enumeration
typedef enum {
    invalid = 1,
    left = 2,
    right = 3,
    forward = 4,
    backward = 5,
    up = 6,
    down = 7,
} orientation_t;

EventGroupHandle_t task1_Event = xEventGroupCreate();

class orient_compute : public scheduler_task
{
    public:
        orient_compute(uint8_t priority) : scheduler_task("compute", 2048, priority)
        {
            /* We save the queue handle by using addSharedObject() */
            QueueHandle_t my_queue = xQueueCreate(1, sizeof(orientation_t));
           // QueueHandle_t my_queue = xQueueCreate(1, sizeof(orient));
            addSharedObject(shared_SensorQueueId, my_queue);
        }
        bool init(void)
        {
            sens.init();
            return true;
        }
        bool run(void *p)
        {
            /* Compute orientation here, and send it to the queue once a second */
            orientation_t orientation;
            int left1 = sens.getX();
            int right1 = sens.getY();
            int up1 = sens.getZ();
            //printf("Here are the values for x, y, z : %i  %i   %i\n",left1, right1, up1);
            //printf("Task being sent\n");
            if((left1 > 0) && (right1 <= 0))
            {
               orientation = left;
            }
            else if ((right1 > 0) && (left1 <= 0))
            {
               orientation = right;
            }
            else if (up1 > 0)
            {
                orientation = up;
            }
            else if (up1 < 0)
            {
                orientation = down;
            }
            else
            {
                orientation = invalid;
            }
            xQueueSend(getSharedObject(shared_SensorQueueId), &orientation, portMAX_DELAY);
            vTaskDelay(1000);
            return true;
        }
    private:
            Acceleration_Sensor& sens = Acceleration_Sensor::getInstance();
};

class orient_process : public scheduler_task
{
    public:
        orient_process (uint8_t priority) : scheduler_task("process", 2048, priority)
        {
            /* Nothing to init */
        }
        bool init()
        {
            LPC_GPIO1-> FIODIR |= (1 << 0);
            LPC_GPIO1-> FIODIR |= (1 << 2);
            LPC_GPIO1-> FIODIR |= (1 << 4);
        }
        bool run(void *p)
        {
            /* We first get the queue handle the other task added using addSharedObject() */
            orientation_t orientation = invalid;
            QueueHandle_t qid = getSharedObject(shared_SensorQueueId);

            /* Sleep the task forever until an item is available in the queue */
            //I'm gonna wait 1000 milliseconds, and you are going to wake me up at anytime during this time if anyone
            //sends me data. If you don't go to else statement, if we had one!!!!!
            if (xQueueReceive(qid, &orientation, portMAX_DELAY))
            {
                printf("Task Received\n");
                switch(orientation)
                {

                case left:
                    printf("The orientation is left\n");
                    BIT(LPC_GPIO1->FIOPIN).b2 = 0;
                    break;
                case right:
                    printf("The orientation is right\n");
                    BIT(LPC_GPIO1->FIOPIN).b2 = 1;
                    break;
                case up:
                     printf("The orientation is up\n");
                     BIT(LPC_GPIO1->FIOPIN).b4 = 0;
                     break;
                 case down:
                     printf("The orientation is down\n");
                     BIT(LPC_GPIO1->FIOPIN).b4 = 1;
                     break;
                }
            }
            return true;
        }
};
class producer_task : public scheduler_task
{
    public:
        producer_task(uint8_t priority) : scheduler_task("producer", 4096, priority){
            QueueHandle_t my_queue = xQueueCreate(1, 4);
            addSharedObject(shared_SensorQueueId, my_queue);
        }

        bool run(void *p)
        {
            int light_value[100];
            int sum = 0;
            int average;

            TickType_t xLastWakeTime;
            const TickType_t xFrequency = 1;

            // Initialise the xLastWakeTime variable with the current time.
            xLastWakeTime = xTaskGetTickCount ();
            int i;
            for(i = 0; i < 100; i++)
            {
                 // Wait for the next cycle.
                 vTaskDelayUntil( &xLastWakeTime, xFrequency );

                 // Perform action here.
                light_value[i] = LS.getRawValue();
                sum += light_value[i];
            }
            average = sum / 100;

            if(i == 100){
                xQueueSend(getSharedObject(shared_SensorQueueId), &average, portMAX_DELAY); //sends queue
            }

            xEventGroupSetBits(task1_Event, 1 << 1);

            return true;
        }

};
class consumer_task : public scheduler_task
{
    public:
        consumer_task(uint8_t priority) : scheduler_task("consumer", 4096, priority){

        }

        bool run(void *p)
        {
            QueueHandle_t qid = getSharedObject(shared_SensorQueueId);
            int avg;
            if(xQueueReceive(qid, &avg, portMAX_DELAY)){
                //u0_dbg_printf("Queue Receive Average: %i\n", average);
                char str[10];
                char str1[10];
                uint64_t uptime_ms = sys_get_uptime_ms();
                sprintf(str, "%i, ", uptime_ms);
                sprintf(str1, "%i\n", avg);
                strcat(str, str1);

                u0_dbg_printf("Appending values\n");
                Storage::append("1:sensor.txt", str, sizeof(str), 0);
            }
            vTaskDelay(1000);
            xEventGroupSetBits(task1_Event, 1 << 2);
            return true;
        }
};
class watchdog_task : public scheduler_task
{
    public:
        watchdog_task (uint8_t priority) : scheduler_task("watchdog", 2048, priority)
        {
            /* Nothing to init */
        }
        bool run(void *p)
        {

//            TickType_t xLastWakeTime;
//            const TickType_t xFrequency = 60;
//
//            // Initialise the xLastWakeTime variable with the current time.
//            xLastWakeTime = xTaskGetTickCount ();
//          for(;;)
//          {
//               // Wait for the next cycle.
//               vTaskDelayUntil( &xLastWakeTime, xFrequency );
//
//               // Perform action here.
//               uint8_t cpu_usage = getTaskCpuPercent();
//               char str[10];
//               sprintf(str, "%i", cpu_usage);
//               Storage::append("1:cpu.txt", str, sizeof(str), 0);
//          }

            uint32_t bit = xEventGroupWaitBits(task1_Event, ((1<<1)|(1<<2)), 1, 1, 1000);
            //u0_dbg_printf("%i\n", bit);

            if(bit == 0)
            {
                char data1[30] = "stuck at producer task\n";
                Storage::append("1:stuck.txt", data1, sizeof(data1)-1,0);
            }
            else if(bit == 4)
            {
                char data2[30] = "stuck at consumer task\n";
                Storage::append("1:stuck.txt", data2, sizeof(data2)-1,0);
            }
            vTaskDelay(1000);
            return true;
        }

};
class StepperMotor : public scheduler_task
{
    public:
    StepperMotor (uint8_t priority) : scheduler_task("watchdog", 2048, priority)
          {
              /* Nothing to init */
          }
    bool run(void *p)
    {
        LPC_PINCON->PINSEL2 &= ~((1<<0)|(1<<1));
        LPC_PINCON->PINSEL2 &= ~((1<<2)|(1<<3));
        LPC_PINCON->PINSEL2 &= ~((1<<4)|(1<<5));
        LPC_PINCON->PINSEL2 &= ~((1<<6)|(1<<7));
        LPC_PINCON->PINSEL2 &= ~((1<<8)|(1<<9));
        LPC_PINCON->PINSEL2 &= ~((1<<10)|(1<<11));

        //LPC_GPIO2-> FIODIR |= (1 << 6);

        BIT(LPC_GPIO2->FIOPIN).b0 = 0;
        BIT(LPC_GPIO2->FIOPIN).b1 = 0;
        BIT(LPC_GPIO2->FIOPIN).b2 = 0;
        BIT(LPC_GPIO2->FIOPIN).b3 = 0;
        BIT(LPC_GPIO2->FIOPIN).b4 = 0;
        BIT(LPC_GPIO2->FIOPIN).b5 = 0;
        return true;
    }
    private:

};
/* At terminal.cpp, add the following code at the taskEntry() function */

/* ----------------------------------------------
 * Your source file, such as "my_source.cpp"
 * We will add our command handler function here
 */
int main(void)
{
    /**
     * A few basic tasks for this bare-bone system :
     *      1.  Terminal task provides gateway to interact with the board through UART terminal.
     *      2.  Remote task allows you to use remote control to interact with the board.
     *      3.  Wireless task responsible to receive, retry, and handle mesh network.
     *
     * Disable remote task if you are not using it.  Also, it needs SYS_CFG_ENABLE_TLM
     * such that it can save remote control codes to non-volatile memory.  IR remote
     * control codes can be learned by typing the "learn" terminal command.
     */
    //scheduler_add_task(new orient_compute(PRIORITY_LOW));
   // scheduler_add_task(new orient_process(PRIORITY_LOW));
    //scheduler_add_task(new producer_task(PRIORITY_MEDIUM));
   // scheduler_add_task(new consumer_task(PRIORITY_MEDIUM));
    //scheduler_add_task(new watchdog_task(PRIORITY_HIGH));
      //scheduler_add_task(new );

    //these are the tasks I can run using my classes.
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
   // scheduler_start();
    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    //scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));
    //scheduler_add_task(new GPIOTask(PRIORITY_CRITICAL));
    //scheduler_add_task(new SSPTask(PRIORITY_CRITICAL));
    //scheduler_add_task(new UARTTask(PRIORITY_CRITICAL));
    //scheduler_add_task(new INTTask(PRIORITY_CRITICAL));

    //scheduler_add_task(new I2CTask(PRIORITY_HIGH));

    /* Change "#if 0" to "#if 1" to run period tasks; @see period_callbacks.cpp */

    #if 0
    scheduler_add_task(new periodicSchedulerTask());
    #endif

    /* The task for the IR receiver */
    // scheduler_add_task(new remoteTask  (PRIORITY_LOW));

    /* Your tasks should probably used PRIORITY_MEDIUM or PRIORITY_LOW because you want the terminal
     * task to always be responsive so you can poke around in case something goes wrong.
     */

    /**
     * This is a the board demonstration task that can be used to test the board.
     * This also shows you how to send a wireless packets to other boards.
     */
    #if 0
        scheduler_add_task(new example_io_demo());
    #endif

    /**
     * Change "#if 0" to "#if 1" to enable examples.
     * Try these examples one at a time.
     */
    #if 0
        scheduler_add_task(new example_task());
        scheduler_add_task(new example_alarm());
        scheduler_add_task(new example_logger_qset());
        scheduler_add_task(new example_nv_vars());
    #endif

    /**
     * Try the rx / tx tasks together to see how they queue data to each other.
     */
    #if 0
        scheduler_add_task(new queue_tx());
        scheduler_add_task(new queue_rx());
    #endif

    /**
     * Another example of shared handles and producer/consumer using a queue.
     * In this example, producer will produce as fast as the consumer can consume.
     */
    #if 0
        scheduler_add_task(new producer());
        scheduler_add_task(new consumer());
    #endif

    /**
     * If you have RN-XV on your board, you can connect to Wifi using this task.
     * This does two things for us:
     *   1.  The task allows us to perform HTTP web requests (@see wifiTask)
     *   2.  Terminal task can accept commands from TCP/IP through Wifly module.
     *
     * To add terminal command channel, add this at terminal.cpp :: taskEntry() function:
     * @code
     *     // Assuming Wifly is on Uart3
     *     addCommandChannel(Uart3::getInstance(), false);
     * @endcode
     */
    #if 0
        Uart3 &u3 = Uart3::getInstance();
        u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
        scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
    #endif

    scheduler_start(); ///< This shouldn't return
    return -1;
}
