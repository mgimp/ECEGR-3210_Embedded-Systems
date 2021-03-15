For Assignment 7: Controlled Movement:
    I'm pretty sure that its possible to make an interrupt that is triggered by encoder pulses.
    I was interested in making another TA3 function that checks for encoder pulses, and this could work if the 
    timer triggers significantly faster than the encoder pulses. I would have to figure out a way to tell if the encoder
    has pulsed but this is certainly possible. An interrupt triggered by encoder pulses would be simpler once I figure 
    out how to implement it. 

    **I've realized that TA3 is already set up as an interrupt. Can new registers be set up that are triggered by
    outside events, or are they restricted to the timed interrupt of TA3.

    pg.789 19.2.4.1 Capture Mode
        Capture inputs CCIxA and CCIxB are connected to external pins or internal
        signals and are selected with the CCIS bit. The CM (capture mode) bits select the capture edge of the
        input signal as rising, falling, or both. A capture occurs on the selected edge of the input signal. 
        If a capture occurs then the timer value is copied into the TAxCCRn register and the interrupt flag CCIFG
        is set.

    pg.795 19.2.6 Timer _A interrupts  

    pg.121 2.4.3.11 IPR0 Register        
        IRQ 0 to 3 Priority Register. Use Interrupt Prority Registers to assign priorities from 0 to 255 to each of the available
        interrupts. 0 is highest and 255 is lowest.

        The hardware priority mechanis only looks at the upper N bit of the priority field (where N is 3 to the MSP432 family 
        meaning bits 7-5 of the priority bit), so any priority myst be performed in those bits. 
        3 bits per register represent priority, essentially meaning we have 8 levels of priority.

        The priority registers are described as PRI_#.
        IPR0 Registers are 4 bytes long and each byte represents a PRI_#.

        Example:
            TA3_0_IRQHandler and TA3_N_IRQHandler use PRI_15 and PRI_14 in IPR[3]
            Setting these registers means activating the the registers at some level of priority, let's say they're both priority level 3
            PRI_15 uses bits 31-29 in IPR[3]
            PRI_14 uses bits 23-21 in IPR[3]
            *in code*
            NVIC->IPR[3] = (NVIC->IPR[3] & 0x0000FFFF) | 0x40400000;

    pg.116 2.4.3.1 ISER0 Register
        The ISER registers are used to enable interrupts on IRQ 0 to 63, meaning PRI_0 tp PRI_63 in registers IPR[0] to IPR[15].
        Make sure to enable interrupts for the desired register after setting it up.

    FROM TI-RSLK-PIN_OUT-README.rtf
        Left Encoder A connected to P10.5 (J5)
        Left Encoder B connected to P5.2 (J2)
        Right Encoder A connected to P10.4 (J5)
        Left Encoder B connected to P5.0 (J2)

        Pololu encoder has 12 counts per revolution (of the gearbox) (counting all 4 edges (rise and fall for A and B encoder pulses)).
        The motor has a gearbox with a 120:1 ratio.
        This gives 12*120 = 1440 counts per revolution of the wheel. Since we are only counting one edge of the encoder,
        we need to divide by 4 for a total of 360 counts per revolution.

    The basic structure of an encoder trigger would be as follows:
    // -----INITIALIZATION-----
        void InDummy(uint16_t t){};
        void (*EncoderTask0)(uint16_t count) = InDummy;
        void (*EncoderTask1)(uint16_t count) = InDummy;

        void EcoderInterrupt_Init(void(*task0)(uint_t count), void(*task1)(uint_t count)){
            EncoderTaskLeft = taskLeft;
            EncoderTaskRight = taskRight;

            // SET UP REGISTERS
            P??->SEL0 == 0x??;
            P??->SEL1 == 0x??;
            P??->DIR == 0x??;

            // MUST FIGURE OUT IF THERE IS A CCR EQUIVILENT FOR INTERRUPTS

            // SET UP INTERRUPT PRIORITY
            NVIC->IP[?] == 0x??;    // priority
            NVIC->ISER[?] == 0x??;    // enable interrupts

            // Make sure interrupt is enabled before leaving intitialization function
        }

        // Handlers acknowledge that the interrupt has happened and execute the task (I think)
        // Left and Right handlers should be structured the same.
        void EncoderHandlerLeft(void){
            // Some Acknowledgement
            (*EncoderTaskLeft)(??);
        }

        void EncoderHandlerRight(void){
            // Some Acknowledgement
            (*EncoderTaskRight)(??);
        }

    // -----ENCODER TASKS-----
        uint_t countLeft;
        uint_t velocityLeft;
        void EncoderCountLeft(){
            // stuff
        }

        uint_t countRight;
        uint_t velocityRight;
        void EncoderCountRight(){
            // stuff
        }

    Basically, the interrupt should trigger whenever the left or right encoder pulses.
    Then it passes a function that counts the encoder pulses so far.
    After some time has passed, the encoder pulses will be used to calculate a velocity.
    The velocity will be compared to the input velocity for the motor functions.
    If the calculated velocity is much different from the input velocity then a new input velocity is 
    delivered.
    Rinse and repeat until all movements have been completed.

    I'll need to initialize TA2 and use the available functions already connected to interrupt priorities for the encoder
    pulse detection.