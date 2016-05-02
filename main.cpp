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


QueueHandle_t qh = 0;

class StepperMotor : public scheduler_task
{
    public:
    StepperMotor (uint8_t priority) : scheduler_task("watchdog", 2048, priority)
    {
              /* Nothing to init */
    }
    bool init(void)
    {
        // Set Pins p2.0 - p2.7 To 00 for GPIO Function
        // Can also just do LPC_PINCON->PINSEL4 &= ~FF;
        LPC_PINCON->PINSEL4 &= ~((1<<0)|(1<<1)); // 2.0 Enable
        LPC_PINCON->PINSEL4 &= ~((1<<2)|(1<<3)); // 2.1 Step
        LPC_PINCON->PINSEL4 &= ~((1<<4)|(1<<5)); // 2.2 MS1
        LPC_PINCON->PINSEL4 &= ~((1<<6)|(1<<7)); // 2.3 MS2
        LPC_PINCON->PINSEL4 &= ~((1<<8)|(1<<9)); // 2.4 MS3
        LPC_PINCON->PINSEL4 &= ~((1<<10)|(1<<11)); // 2.5 RESET
        LPC_PINCON->PINSEL4 &= ~((1<<12)|(1<<13)); // 2.6 DIR
        LPC_PINCON->PINSEL4 &= ~((1<<14)|(1<<15)); // 2.7 SLEEP

        /* Set p2.0 - p2.7 to outputs (1) */
        // Can also just do LPC_GPIO2->FIODIR |= FF; Here (same thing) i believe;
        LPC_GPIO2->FIODIR |= (1 << 0);
        LPC_GPIO2->FIODIR |= (1 << 2);
        LPC_GPIO2->FIODIR |= (1 << 4);
        LPC_GPIO2->FIODIR |= (1 << 6);
        LPC_GPIO2->FIODIR |= (1 << 8);
        LPC_GPIO2->FIODIR |= (1 << 10);
        LPC_GPIO2->FIODIR |= (1 << 12);
        LPC_GPIO2->FIODIR |= (1 << 14);
        return true;
    }
    bool run(void *p)
    {
        int steps = 0;


        BIT(LPC_GPIO2->FIOPIN).b11_10 = 1;

        //Waits for a message
        if (xQueueReceive(qh,&steps,portMAX_DELAY))
        {

            u0_dbg_printf("We have %i steps from the queue receive\n", steps);

            BIT(LPC_GPIO2->FIOPIN).b3_2 = 1;
            BIT(LPC_GPIO2->FIOPIN).b1_0 = 1;
            BIT(LPC_GPIO2->FIOPIN).b13_12 = 1;

            for(int b = 0; b < steps;b++)
            {
                BIT(LPC_GPIO2->FIOPIN).b9_8 = 0;
                BIT(LPC_GPIO2->FIOPIN).b7_6 = 0;
                BIT(LPC_GPIO2->FIOPIN).b5_4 = 0;
            }

        }
        BIT(LPC_GPIO2->FIOPIN).b15_14 = 1;


        /*
        Return to starting position of stepper for next use?
        */

        // put "Z" = DONE on UART line so android device is able to send more requests
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

        //Enable UART3 PCLK
        LPC_SC->PCLKSEL1 &= ~(3 << 18); // Clear Values for UART 3 Clock
        LPC_SC->PCLKSEL1 |= (1 << 18); // Peripheral Clock

        //Enable DLAB
        LPC_UART3->LCR |= (1 << 7);

        //Sets to 9600 baudrate to match bluetooth
        uint16_t DL = sys_get_cpu_clock()/(9600 * 16);
        LPC_UART3->DLM = DL >> 8;
        LPC_UART3->DLL = DL;

        LPC_PINCON->PINSEL0 &= ~(3 << 0); // Resets Value for pin 0.0
        LPC_PINCON->PINSEL0 &= ~(3 << 2); // Resets Value for pin 0.1

        LPC_PINCON->PINSEL0 != (2 << 0); // Sets 0.0 to 10 for TXD3
        LPC_PINCON->PINSEL0 != (2 << 2); // Sets 0.1 to 10 for RXD3

        LPC_UART3->LCR = 3; //8-bit data and Disabled DLAB

        qh = xQueueCreate( 1 , sizeof(int)); // Creates Queue of Size 1

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
        int steps;
        char ch = uart3_getchar(); // Sleeps Till FIFO Not Empty

        //decode message = how many steps on motor
        switch(ch){
            case 'a':
                steps = 1;
                u0_dbg_printf("%i steps\n", steps);  // Translate data a as 1 step
                break;
            case 'b':
                steps = 10;
                u0_dbg_printf("%i steps\n", steps); // Translate data b as 10 step
                break;
            case 'c':
                steps = 100;
                u0_dbg_printf("%i steps\n", steps); // Translate data c as 100 step
                break;
            case 'd':
                steps = 1000;
                u0_dbg_printf("%i steps\n", steps); // Translate data d as 100 step
                break;
            default:
                //Error
                break;
        }

        //Will basically discard values received from UART3 if not lowercase a,b,c,d
        if (steps == 1 || steps == 10 || steps == 100 || steps == 1000){
        xQueueSend(qh, &steps, portMAX_DELAY); // Sends the steps to the stepper task
        }

        return true;
    }
    private:

};


int main(void)
{
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
