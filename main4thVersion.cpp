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
        // Set Pins p2.0 - p2.7 To 00 for GPIO Function
        // Can also just do LPC_PINCON->PINSEL4 &= ~FF;
        LPC_PINCON->PINSEL4 &= ~((1<<0)|(1<<1)); // 2.0 en
        LPC_PINCON->PINSEL4 &= ~((1<<2)|(1<<3)); // 2.1 S1
        LPC_PINCON->PINSEL4 &= ~((1<<4)|(1<<5)); // 2.2 s2
		LPC_PINCON->PINSEL4 &= ~((1<<6)|(1<<7)); // 2.3 s3
		LPC_PINCON->PINSEL4 &= ~((1<<8)|(1<<9)); // 2.4 stp
		LPC_PINCON->PINSEL4 &= ~((1<<10)|(1<<11)); // 2.5 dir

        /* Set p2.0 - p2.7 to outputs (1) */
        // Can also just do LPC_GPIO2->FIODIR |= FF; Here (same thing) i believe;
        LPC_GPIO2->FIODIR |= (1 << 0);
        LPC_GPIO2->FIODIR |= (1 << 1);
        LPC_GPIO2->FIODIR |= (1 << 2);
        LPC_GPIO2->FIODIR |= (1 << 3);
        LPC_GPIO2->FIODIR |= (1 << 4);
		LPC_GPIO2->FIODIR |= (1 << 5);

		LPC_GPIO2->FIOCLR = (1<<0);
		LPC_GPIO2->FIOCLR = (1<<1);
		LPC_GPIO2->FIOCLR = (1<<2);
		LPC_GPIO2->FIOCLR = (1<<3);
		LPC_GPIO2->FIOCLR = (1<<4);
		LPC_GPIO2->FIOSET = (1<<5); //set dir to 1 = outward 0 = inward


        return true;
    }
    bool run(void *p)
    {
        int steps = 0;

        if (xQueueReceive(qh,&steps,portMAX_DELAY))
        {

			u0_dbg_printf("We have %i steps from the queue receive\n", steps);

			//set 2.0 high

			for(int b = 0; b < steps;b++)
			{
			//2.1 high
			LPC_GPIO2->FIOSET = (1<<4);
			vTaskDelay(1);
			LPC_GPIO2->FIOCLR = (1<<4);
			vTaskDelay(1);
			//delay
			//2.1 low
			//delay
			}
        }
        u0_dbg_printf("Done\n");
        //vTaskDelay(5000);

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
		char ch = uart3_getchar();
		//u0_dbg_printf("%c\n", ch);
		switch(ch){
			case 'a':
				steps = 1;
				u0_dbg_printf("%i steps\n", steps);
				break;
			case 'b':
				steps = 10;
				u0_dbg_printf("%i steps\n", steps);
				break;
			case 'c':
				steps = 100;
				u0_dbg_printf("%i steps\n", steps);
				break;
			case 'd':
				steps = 1000;
				u0_dbg_printf("%i steps\n", steps);
				break;
			case 'e':
				LPC_GPIO2->FIOCLR = (1<<5); //set dir to 1 = outward 0 = inward
				u0_dbg_printf("Direction inward\n");
				break;
			case 'f':
				LPC_GPIO2->FIOSET = (1<<5); //set dir to 1 = outward 0 = inward
				u0_dbg_printf("Direction outward\n");
				break;
			default: break;
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
