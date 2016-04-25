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
    bool run(void *p)
    {
        int steps = 0;
        if (xQueueReceive(qh,&steps,portMAX_DELAY) ){
            for (int i = 0 ; i < steps ; i++){
                //do one step by changing voltages here
            }
        }
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
        //previous initialized variables
        //uint32_t baud =38400;
        // uint8_t dll;


        //powering up UART2
        LPC_SC->PCONP |= (1 << 24);


        //PCLKSEL for UART2 and 18+19(whatever that is!)

        LPC_SC->PCLKSEL1 &= ~(3 << 16);
        LPC_SC->PCLKSEL1 &= ~(3 << 18);
        LPC_SC->PCLKSEL1 |= (1 << 16);
        LPC_SC->PCLKSEL1 |= (1 << 18);


        //PinSelect UART2  2.8 , 2.9
        LPC_PINCON->PINSEL4 &= ~(0xF << 16); // Clear values
        LPC_PINCON->PINSEL4 |= (0xA << 16); // Set values for UART2 Rx/Tx

        //dll value, New settings for bluetooth 4/24
        //CLK is 48mhz, Desired baudrate = 9600 for bluetooth
        //UArt BaudRate = ( PCLK / 16 * DLL)
        // DLL = CLK / (16 * baudrate)

        uint16_t DL = (48000000 / (16 * 9600)); // Change baudrate to bleutooth


        LPC_UART3->LCR = (1<<7);

        LPC_UART2->DLM = DL >> 8; //upper 8 bits of DL
        LPC_UART2->DLL = DL; //lower 8 bits of DL
        LPC_UART2->LCR = 3; //Allows for 8 Bit Data & Disables DLAB

        // Here are the previous settings
        //dll = sys_get_cpu_clock()/(16 * baud);
        //LPC_UART3->DLL = dll;
        //LPC_UART3->DLM = 0;
        //LPC_UART3->LCR = 0x03;

        qh = xQueueCreate( 1 , sizeof(int)); // Creates Queue of Size 1

        return true;
    }
    char uart2_putchar(char out)
    {
        LPC_UART2->THR = out;
        while(!(LPC_UART3->LSR & (1 << 5)));
        return 1;
    }
    char uart2_getchar(void)
    {
        while(LPC_UART2->LSR & (1 << 0))
        {
            break;
        }
        char c = LPC_UART2->RBR;
        return c;
    }
    bool run(void *p)
    {

        char receive;
        int steps = 0;

        receive = uart2_getchar(); //wait for data from app

        if (receive == 'A'){ // 1 Step
            steps = 1;
        }
        else if (receive == 'B'){ // 5 Steps
            steps = 5;
        }
        else if (receive == 'C'){ // 25 Steps
            steps = 25;
        }
        else if (receive == 'D'){ // 125 Steps
            steps = 125;
        }
        else if (receive == 'E'){ // 500 Steps
            steps = 500;
        }
        else if (receive == 'F'){ // 2500 Steps
            steps = 2500;
        }
        else{
            steps = 0;
        }

        xQueueSend(qh, &steps, portMAX_DELAY); // Sends the steps to the stepper task

        //decode message = how many steps on motor
        //send amount of steps to motor 0 256 steps
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


    //these are the tasks I can run using my classes.
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
