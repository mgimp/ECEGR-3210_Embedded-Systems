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
            
        }

        uint_t countRight;
        uint_t velocityRight;
        void EncoderCountRight(){

        }

    Basically, the interrupt should trigger whenever the left or right encoder pulses.
    Then it passes a function that counts the encoder pulses so far.
    After some time has passed, the encoder pulses will be used to calculate a velocity.
    The velocity will be compared to the input velocity for the motor functions.
    If the calculated velocity is much different from the input velocity then a new input velocity is 
    delivered.
    Rinse and repeat until all movements have been completed.