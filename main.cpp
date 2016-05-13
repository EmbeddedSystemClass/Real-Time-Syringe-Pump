#include <stdio.h>
#include "LPC17xx.h"
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "printf_lib.h"
//these are for I2C lab
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "io.hpp"
#include "event_groups.h"
#include "command_handler.hpp"

//Calculate how many steps per ml of liquid
#define SYRINGE_VOL_ML 30.0 // How much liquid our syringe holds in ML
#define SYRINGE_BARREL_LEN_MM 50.0 // Length of SYringe Barrel in MM

#define T_ROD_PITCH_MM 1.25 // 1.25mm Pitch of Threaded Rod
#define STEPS_PER_REV 200.0 // Number of Steps to make a revolution
#define USTEPS_PER_STEP 16.0 // 16 microsteps = 1 step

//How many steps it takes to dispense one mL
long stepsPerML = (USTEPS_PER_STEP * STEPS_PER_REV * SYRINGE_BARREL_LEN_MM) / (SYRINGE_VOL_ML * T_ROD_PITCH_MM );

//4266.66666667 steps per mL


//Queue to start stepper motor task
QueueHandle_t startStep = 0;
//Semaphore to prevent input while running
//SemaphoreHandle_t appLock = xSemaphoreCreateMutex();

class StepperMotor : public scheduler_task
{
    public:
    StepperMotor (uint8_t priority) : scheduler_task("StepperMotor", 2048, priority)
    {
              /* Nothing to init */
    }
    bool init(void)
    {
        // Set Pins p2.0 - p2.7 To 00 for GPIO Function
        // Can also just do LPC_PINCON->PINSEL4 &= ~FF;
        LPC_PINCON->PINSEL4 &= ~((1<<0)|(1<<1)); // 2.0 Dir
        LPC_PINCON->PINSEL4 &= ~((1<<2)|(1<<3)); // 2.1 Step
        LPC_PINCON->PINSEL4 &= ~((1<<4)|(1<<5)); // 2.2 ms1
        LPC_PINCON->PINSEL4 &= ~((1<<6)|(1<<7)); // 2.3 ms2
        LPC_PINCON->PINSEL4 &= ~((1<<8)|(1<<9)); // 2.4 ms3
        LPC_PINCON->PINSEL4 &= ~((1<<10)|(1<<11)); // 2.5 en

        /* Set p2.0 - p2.7 to outputs (1) */
        // Can also just do LPC_GPIO2->FIODIR |= FF; Here (same thing) i believe;
        LPC_GPIO2->FIODIR |= (1 << 0);
        LPC_GPIO2->FIODIR |= (1 << 1);
        LPC_GPIO2->FIODIR |= (1 << 2);
        LPC_GPIO2->FIODIR |= (1 << 3);
        LPC_GPIO2->FIODIR |= (1 << 4);
        LPC_GPIO2->FIODIR |= (1 << 5);

        LPC_GPIO2->FIOCLR = (1<<0); // 2.0 Dir starts low goes outward
        LPC_GPIO2->FIOCLR = (1<<2); // 2.2 ms1 low
        LPC_GPIO2->FIOCLR = (1<<3); // 2.3 ms2 low
        LPC_GPIO2->FIOCLR = (1<<4); // 2.4 ms3 low
        LPC_GPIO2->FIOCLR = (1<<5); // 2.5 enable low - allows motor control

        return true;
    }

    char uart3_putchar(char out)
    {
        LPC_UART3->THR = out;
        while(!(LPC_UART3->LSR & (1 << 5)));
        return 1;
    }

    bool run(void *p)
    {
        int steps = 0;

        if (xQueueReceive(startStep,&steps,portMAX_DELAY))
        {
            u0_dbg_printf("We have %i steps from the queue receive\n", steps);

            for(int b = 0; b < steps;b++) // Dispense liquid w/ # steps
            {
            //Motor does 1 step every low to high transition
            LPC_GPIO2->FIOSET = (1<<1); //2.1 high
            vTaskDelay(1); //delay
            LPC_GPIO2->FIOCLR = (1<<1); //2.1 low
            vTaskDelay(1); //delay
            }

            LPC_GPIO2->FIOSET = (1<<0); // Reverse direction to return to starting position
            for(int b = 0; b < steps;b++)
            {
              //Motor does 1 step every low to high transition
              LPC_GPIO2->FIOSET = (1<<1); //2.1 high
              vTaskDelay(1); //delay
              LPC_GPIO2->FIOCLR = (1<<1); //2.1 low
              vTaskDelay(1); //delay
            }
            LPC_GPIO2->FIOCLR = (1<<0); // Set direction back to outward for next use;
        }
        u0_dbg_printf("Done\n");

        uart3_putchar('y'); // Sends "DONE" signal to app

        /*
        Return to starting position of stepper for next use?
        */
        return true;
    }
    private:

};

class BluetoothTask : public scheduler_task
{
    public:
      BluetoothTask (uint8_t priority) : scheduler_task("watchdog", 2048, priority)
    {
              /* Nothing to init */
    }
    bool init(void)
    {
        LPC_SC->PCONP |= (1 << 25); // Power Enable UART3
        LPC_SC->PCLKSEL1 &= -(3 << 18);

        BIT(LPC_SC->PCLKSEL1).b19_18 = 1; // Peripheral Clock
        BIT(LPC_UART3->LCR).b7 = 1; // Enable DLAB

        uint16_t DL = sys_get_cpu_clock()/(9600 * 16);
        LPC_UART3->DLM = DL >> 8;
        LPC_UART3->DLL = DL;

        LPC_PINCON->PINSEL0 &= -(3 << 0);
        LPC_PINCON->PINSEL0 &= -(3 << 2);

        BIT(LPC_PINCON->PINSEL0).b1_0 = 2; // TXD3 P0.0
        BIT(LPC_PINCON->PINSEL0).b3_2 = 2; // RXD3 P0.1

        LPC_UART3->LCR = 3; //8-bit data

        BIT(LPC_UART3->LCR).b7 = 0; // Disable DLAB

        startStep = xQueueCreate( 1 , sizeof(int)); // Creates Queue of Size 1

        return true;
    }

    char uart3_getchar(void){
        while(!(LPC_UART3->LSR & (1 << 0))); // wait for FIFO to not be empty
        char c = LPC_UART3->RBR;
        return c;
    }

    char uart3_putchar(char out)
    {
        LPC_UART3->THR = out;
        while(!(LPC_UART3->LSR & (1 << 5)));
        return 1;
    }

    bool run(void *p)
    {

        double mlLiquid = 0;
        char ch, ignoreChar;
        int steps = 0;
        bool done = false;

        ch = 0;
        ch = uart3_getchar();
        if (ch == 'a'){ // wait for start byte
            mlLiquid = 0;
            while (!done){ // add liquid until end 'z' signal // grabs byte
                    ch = uart3_getchar();
                    switch(ch){
                             case 'b':
                                 mlLiquid += .05; // .05 mL increment
                                 u0_dbg_printf("Total Liquid: %d\n", mlLiquid);
                                 break;
                             case 'c':
                                 mlLiquid += .1; // .1 mL increment
                                 u0_dbg_printf("Total Liquid: %d\n", mlLiquid);
                                 break;
                             case 'd':
                                 mlLiquid += .5; // .5 mL increment
                                 u0_dbg_printf("Total Liquid: %d\n", mlLiquid);
                                 break;
                             case 'e':
                                 mlLiquid += 1; // 1 mL increment
                                 u0_dbg_printf("Total Liquid: %d\n", mlLiquid);
                                 break;
                             case 'f':
                                 mlLiquid += 5; // 5 mL increment
                                 u0_dbg_printf("Total Liquid: %d\n", mlLiquid);
                                 break;
                             case 'g':
                                 mlLiquid += 10; // 10 mL increment
                                 u0_dbg_printf("Total Liquid: %d\n", mlLiquid);
                                 break;
                             case 'z': // End signal
                                 done = true;
                                 break;
                             default: break; // discards all other chars
                    }
            }
            steps = mlLiquid * stepsPerML; //Translate liquid to steps here
            xQueueSend(startStep, &steps, portMAX_DELAY); // Sends the steps to the stepper task
            done = false;
        }
        else { //ignoreChar
            ignoreChar = uart3_getchar();
        }
        return true;
    }
    private:

};


int main(void)
{
    //Relevant Project Tasks
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    scheduler_add_task(new StepperMotor(PRIORITY_MEDIUM));
    scheduler_add_task(new BluetoothTask(PRIORITY_LOW));

    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */

    /* Change "#if 0" to "#if 1" to run period tasks; @see period_callbacks.cpp */

    #if 0
    scheduler_add_task(new periodicSchedulerTask());
    #endif

    /* The task for the IR receiver */
    // scheduler_add_task(new remoteTask  (PRIORITY_LOW));

    #if 0
        scheduler_add_task(new example_io_demo());
    #endif

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
