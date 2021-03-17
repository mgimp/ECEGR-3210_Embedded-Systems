// TA3InputCapture.c
// Runs on MSP432
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P8.2 (TA3CCP2) and call user
// functions.
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1) and call user
// functions.
// Daniel Valvano
// July 11, 2019

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// external signal connected to P10.5 (TA3CCP1) (trigger on rising edge)
// external signal connected to P10.4 (TA3CCP0) (trigger on rising edge)

#include <stdint.h>
#include "msp.h"

void ta3dummy(uint16_t t){};       // dummy function
void (*CaptureTaskA3_0)(uint16_t currenttime) = ta3dummy;// user function
void (*CaptureTaskA3_1)(uint16_t currenttime) = ta3dummy;// user function

//------------TimerA3Capture_Init01------------
// I'm setting TA3 up to interrupt when Left or Right encoders A go high
// They will call functions from Tachmain.c

// Initialize Timer A3 in edge time mode to request interrupts on
// the rising edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1).  The
// interrupt service routines acknowledge the interrupt and call
// a user function.
// Input: task0 is a pointer to a user function called when P10.4 (TA3CCP0) edge occurs
//              parameter is 16-bit up-counting timer value when P10.4 (TA3CCP0) edge occurred (units of 0.083 usec)
//        task1 is a pointer to a user function called when P10.5 (TA3CCP1) edge occurs
//              parameter is 16-bit up-counting timer value when P10.5 (TA3CCP1) edge occurred (units of 0.083 usec)
// Output: none
// Assumes: low-speed subsystem master clock is 12 MHz
void TimerA3Capture_Init01(void(*task0)(uint16_t currenttime), void(*task1)(uint16_t currenttime)){
    CaptureTaskA3_0 = task0;               // user function for left wheel
    CaptureTaskA3_1 = task1;               // user function for right wheel

    // initialize P10.4,5 and make them GPIO In to trigger CCR0 and 1 during encoder pulses
    // I'm pretty sure that P10.4,5 hold the values for the encoders and should trigger the interrupts
    P10->SEL0 &= ~0x30;
    P10->SEL1 &= ~0x30;                   // configure P10.4 as TA3CCIOA and P10.5 as TA3CCI1A
    P10->DIR &= ~0x30;                    // make P10.4,5 in
    TIMER_A3->CTL &= ~0x0030;

    // -----TA3 CTL INSTRUCTIONS-----
    // halt Timer A3
    // bits15-10=XXXXXX, reserved
    // bits9-4,       clock source to SMCLK, input clock divider /1, MC=stop mode
    // bit3=X,           reserved
    // bit2=0,           set this bit to clear
    // bit1=0,           interrupt disable
    // bit0=0,           clear interrupt pending

    // THE FOLLOWING was suggested by Dr. Moser. He said to do 0x0300 instead of 0x0200 in order to establish CTL clock, but the TRM says it should be 0b10 for bits 9-8 in order to set SMCLK.
    TIMER_A3->CTL =(TIMER_A3->CTL & 0xFC08) | 0x0200;       // I EDITED THIS; ORIGINAL: TIMER_A3->CTL = 0x0204; The purpose was to not touch bits 15-10 or 3

    // -----TA3 CCTL INSTRUCTIONS-----
    // bits15-14     capture on rising edge,
    // bits13-12     capture/compare input on CCI0A and CCI1A
    // bit11          synchronous capture source
    // bit10=X,          synchronized capture/compare input
    // bit9=X,           reserved
    // bit8           capture mode
    // bits7-5=XXX,      output mode
    // bit4           enable capture/compare interrupt
    // bit3=X,           read capture/compare input from here
    // bit2=X,           output this value in output mode 0
    // bit1=X,           capture overflow status
    // bit0           clear capture/compare interrupt pending
    // [0100][1001][0001][0000]
    // 0x4910

    // THE FOLLOWING change was suggested by Dr. Moser: Change 0x4911 to 0x4910. He wasn't sure if this would be effective or not.
    TIMER_A3->CCTL[0] = (TIMER_A3->CCTL[0] & 0x0208) | 0x4910;  // I EDITED THIS; ORIGINAL: TIMER_A3->CCTL[0] = 0x4911;
    TIMER_A3->CCTL[1] = (TIMER_A3->CCTL[1] & 0x0208) | 0x4910;  // do both CCTL[0] and CCTL[1] exactly the same
    TIMER_A3->EX0 &= ~0x0007;       // configure for input clock divider /1
    NVIC->IP[3] = (NVIC->IP[3]&0x0000FFFF)|0x30300000; // enable PRI_14 and PRI_15 for the IRQHandler functions
    // interrupts enabled in the main program after all devices initialized
    NVIC->ISER[0] = 0x0000C000; // enable interrupts

    // -----START TIMER-----
    // bits15-10=XXXXXX, reserved
    // bits9-8       clock source to SMCLK
    // bits7-6,       input clock divider /1
    // bits5-4       continuous count up mode
    // bit3=X,           reserved
    // bit2=1,           set this bit to clear
    // bit1=0,           interrupt disable (no interrupt on rollover)
    // bit0=0,           clear interrupt pending
    TIMER_A3->CTL |= 0x0024;       // reset and start Timer A3 in continuous up mode
}

void TA3_0_IRQHandler(void){
    TIMER_A3->CCTL[0] &= ~0x0001;             // acknowledge capture/compare interrupt 0 (turns off the activation flag)
     (*CaptureTaskA3_0)(TIMER_A3->CCR[0]);         // execute user task
}

// THE FOLLOWING was changed on Dr. Moser's suggestion to make TA3_N_IRQHandler just like the previous function, but for for CaptureTaskA3_1.
void TA3_N_IRQHandler(void){
    // make this like TA3_0_IRQHandler, but for Capture/Compare 1, not 0
    TIMER_A3->CCTL[1] &= ~0x0001;             // acknowledge capture/compare interrupt 0
     (*CaptureTaskA3_1)(TIMER_A3->CCR[1]);         // execute user task
}

// old robot code

//------------TimerA3Capture_Init02------old robot version------
// Initialize Timer A3 in edge time mode to request interrupts on
// the rising edges of P10.4 (TA3CCP0) and P8.2 (TA3CCP2).  The
// interrupt service routines acknowledge the interrupt and call
// a user function.
// Input: task0 is a pointer to a user function called when P10.4 (TA3CCP0) edge occurs
//              parameter is 16-bit up-counting timer value when P10.4 (TA3CCP0) edge occurred (units of 0.083 usec)
//        task2 is a pointer to a user function called when P8.2 (TA3CCP2) edge occurs
//              parameter is 16-bit up-counting timer value when P8.2 (TA3CCP2) edge occurred (units of 0.083 usec)
// Output: none
// Assumes: low-speed subsystem master clock is 12 MHz
void TimerA3Capture_Init02(void(*task0)(uint16_t time), void(*task2)(uint16_t time)){
// old robot code
}
