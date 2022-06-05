/*******************************************************************************
* Copyright (C) 2016, Fujitsu Electronics Europe GmbH or a                     *
* subsidiary of Fujitsu Electronics Europe GmbH.                               *
*                                                                              *
* This software, including source code, documentation and related materials    *
* ("Software") is developed by Fujitsu Electronics Europe GmbH ("Fujitsu")     *
* unless identified differently hereinafter for Open Source Software.          *
* All rights reserved.                                                         *
*                                                                              *
* This software is provided free of charge and not for sale, providing test    *
* and sandbox applications. Fujitsu reserves the right to make changes to      *
* the Software without notice. Before use please check with Fujitsu            *
* for the most recent software.                                                *
*                                                                              *
* If no specific Open Source License Agreement applies (see hereinafter),      *
* Fujitsu hereby grants you a personal, non-exclusive,                         *
* non-transferable license to copy, modify and compile the                     *
* Software source code solely for use in connection with products              *
* supplied by Fujitsu. Any reproduction, modification, translation,            *
* compilation, or representation of this Software requires written             *
* permission of Fujitsu.                                                       *
*                                                                              *
* NO WARRANTY: This software is provided �as-is� with no warranty of any kind  *
* (German �Unter Ausschluss jeglicher Gew�hleistung�), express or implied,     *
* including but not limited to non-infringement of third party rights,         *
* merchantability and fitness for use.                                         *
*                                                                              *
* In the event the software deliverable includes the use of                    *
* open source components, the provisions of the respective governing           *
* open source license agreement shall apply with respect to such software      *
* deliverable. Open source components are identified in the read-me files      *
* of each section / subsection.                                                *
*                                                                              *
* German law applies with the exclusion of the rules of conflict of law.       *
*                                                                              *
*                                      ***                                     *
* September 2016                                                               *
* FUJITSU ELECTRONICS Europe GmbH                                              *
*                                                                              *
*******************************************************************************/
/******************************************************************************/
/** \file main.c
 **
 ** \brief UART + ADC for Apollo3
 **
 ** Main Module
 **
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mcu.h"
#include "base_types.h"
#include "stdio.h"

/******************************************************************************/
/* Prototype functions                                                        */
/******************************************************************************/

void ConfigureBaudrate(UART0_Type* pstcUart,uint32_t u32Baudrate, uint32_t u32UartClkFreq);
void PutCharUart(UART0_Type* pstcUart, uint8_t u8Char);
void PutStringUart(UART0_Type* pstcUart, char_t *pu8Buffer);
uint8_t GetCharUart(UART0_Type* pstcUart);
void UartInit(UART0_Type* pstcUart,uint32_t u32Baudrate);
static volatile uint32_t u32Counter;  //ms counter
void delay(uint32_t delayMs);


/******************************************************************************/
/* Procedures and functions                                                   */
/******************************************************************************/
void SysTick_Handler(void)
{
	u32Counter++;
}

void delay(uint32_t delayMs)
{
    uint32_t u32End = u32Counter;
    u32End += delayMs;
    while(u32End != u32Counter) __NOP();
}
/**


 ******************************************************************************
 ** \brief  Init UART...
 **
 ** \param  pstcUart         UART pointer
 **
 ** \param  u32Baudrate      Baudrate
 **
 *****************************************************************************/
void UartInit(UART0_Type* pstcUart,uint32_t u32Baudrate)
{
    //
    // Enable UART clocking
    //
  
    PWRCTRL->DEVPWREN |= (1 << PWRCTRL_DEVPWREN_PWRUART0_Pos);

    while(PWRCTRL->DEVPWRSTATUS_b.HCPA == 0) __NOP();

    //
    // Enable clock / select clock...
    //
    pstcUart->CR = 0;
    pstcUart->CR_b.CLKEN = 1;                 //enable clock
    pstcUart->CR_b.CLKSEL = 1;                //use 24MHz clock
    
    //
    // Disable UART before config...
    //
    pstcUart->CR_b.UARTEN = 0;                //disable UART
    pstcUart->CR_b.RXE = 0;                   //disable receiver
    pstcUart->CR_b.TXE = 0;                   //disable transmitter
    
    
    //
    // Starting UART config...
    //
    
    // initialize baudrate before all other settings, otherwise UART will not be initialized
    SystemCoreClockUpdate();
    ConfigureBaudrate(pstcUart,u32Baudrate,24000000UL);
    
    // initialize line coding...
    pstcUart->LCRH = 0;
    pstcUart->LCRH_b.WLEN = 3;                //3 = 8 data bits (2..0 = 7..5 data bits)
    pstcUart->LCRH_b.STP2 = 0;                //1 stop bit
    pstcUart->LCRH_b.PEN = 0;                 //no parity
    
    //
    // Enable UART after config...
    //
    pstcUart->CR_b.UARTEN = 1;                //enable UART
    pstcUart->CR_b.TXE = 1;                   //enable transmitter
}

/**
 ******************************************************************************
 ** \brief  Set baudrate
 **
 ** \param  pstcUart         UART pointer
 **
 ** \param  u32Baudrate      Baudrate
 **
 ** \param  u32UartClkFreq   UART clock
 **
 *****************************************************************************/
void ConfigureBaudrate(UART0_Type* pstcUart,uint32_t u32Baudrate, uint32_t u32UartClkFreq)
{
    uint64_t u64FractionDivisorLong;
    uint64_t u64IntermediateLong;
    uint32_t u32IntegerDivisor;
    uint32_t u32FractionDivisor;
    uint32_t u32BaudClk;
    
    //
    // Calculate register values.
    //
    u32BaudClk = 16 * u32Baudrate;
    u32IntegerDivisor = (uint32_t)(u32UartClkFreq / u32BaudClk);
    u64IntermediateLong = (u32UartClkFreq * 64) / u32BaudClk;
    u64FractionDivisorLong = u64IntermediateLong - (u32IntegerDivisor * 64);
    u32FractionDivisor = (uint32_t)u64FractionDivisorLong;
    
    //
    // Integer divisor MUST be greater than or equal to 1.
    //
    if(u32IntegerDivisor == 0)
    {
        //
        // Spin in a while because the selected baudrate is not possible.
        //
        while(1);
    }
    //
    // Write the UART regs.
    //
    pstcUart->IBRD = u32IntegerDivisor;
    pstcUart->IBRD = u32IntegerDivisor;
    pstcUart->FBRD = u32FractionDivisor;
}

/**
 ******************************************************************************
 ** \brief  sends a single character  (no timeout !)
 **
 ** \param  pstcUart  UART pointer
 **
 ** \param  u8Char    Data to send
 **
 *****************************************************************************/
void PutCharUart(UART0_Type* pstcUart, uint8_t u8Char)
{
    while(pstcUart->FR_b.TXFF) __NOP();
    pstcUart->DR = u8Char;
}

/**
 ******************************************************************************
 ** \brief  sends a complete string (0-terminated) 
 **
 ** \param  pstcUart  UART pointer
 **
 ** \param  Pointer to (constant) file of bytes in mem
 **
 *****************************************************************************/
void PutStringUart(UART0_Type* pstcUart, char_t *pu8Buffer)
{
  while (*pu8Buffer != '\0')
  { 
    PutCharUart(pstcUart,*pu8Buffer++);        // send every char of string
  }
}

/**
 ******************************************************************************
 ** \brief  Main function
 **
 ** \return int return value, if needed
 ******************************************************************************/
int main(void)
{
		uint32_t u32Cfg;
    uint32_t u32Data;
		char data[10];
    SystemCoreClockUpdate();                //update clock variable SystemCoreClock (defined by CMSIS)
    SysTick_Config(SystemCoreClock / 1000); //setup 1ms SysTick (defined by CMSIS)
    //application initialization area

    GPIO->PADKEY = 0x73; //unlock GPIO configuration
    GPIO->PADREGI_b.PAD32FNCSEL = GPIO_PADREGI_PAD32FNCSEL_ADCSE4; //select ADC4 at PAD32
    GPIO->PADKEY = 0;

    //
    // Enable power for ADC and wait power is enabled
    //
    PWRCTRL->DEVPWREN_b.PWRADC = 1;
    while(PWRCTRL->ADCSTATUS_b.ADCPWD == 0) __NOP();

    ADC->CFG = 0;


    u32Cfg =      _VAL2FLD(ADC_CFG_CKMODE,ADC_CFG_CKMODE_LLCKMODE)   //Low Latency Clock Mode
               |  _VAL2FLD(ADC_CFG_CLKSEL,ADC_CFG_CLKSEL_HFRC)       //HFRC clock
               |  _VAL2FLD(ADC_CFG_TRIGSEL,ADC_CFG_TRIGSEL_SWT)      //Software trigger
               |  _VAL2FLD(ADC_CFG_REFSEL,ADC_CFG_REFSEL_INT2P0)     //internal 2V reference
               |  _VAL2FLD(ADC_CFG_LPMODE,ADC_CFG_LPMODE_MODE1)      //Low Power Mode 1
               |  _VAL2FLD(ADC_CFG_RPTEN,ADC_CFG_RPTEN_SINGLE_SCAN)  //Single scan
               |  _VAL2FLD(ADC_CFG_ADCEN,1);                         //Enable ADC

    //
    // Using slot 0
    //
    ADC->SL0CFG = 0; 
    ADC->SL0CFG_b.CHSEL0 = ADC_SL0CFG_CHSEL0_SE4; //use ADCSE8 as input;
    ADC->SL0CFG_b.ADSEL0 = ADC_SL0CFG_ADSEL0_AVG_1_MSRMT; //Avaerage over 16 measurements
    ADC->SL0CFG_b.PRMODE0 = ADC_SL0CFG_PRMODE0_P14B; //use 14-bit 1.2MS/s
    ADC->SL0CFG_b.WCEN0 = 0; //window comparator mode disabled
    ADC->SL0CFG_b.SLEN0 = 1; //slot enabled

    ADC->WULIM_b.ULIM = 0; //Window comparator upper limit (not used)
    ADC->WLLIM_b.LLIM = 0; //Window comparator lower limit (not used)

    ADC->INTEN_b.WCINC = 1;               //enable window comparator voltage incursion interrupt
    ADC->INTEN_b.WCEXC = 1;               //enable window comparator voltage excursion interrupt
    ADC->INTEN_b.FIFOOVR1 = 1;            //enable FIFO 100% full interrupt
    ADC->INTEN_b.FIFOOVR2 = 1;            //enable FIFO 75% full interrupt
    ADC->INTEN_b.SCNCMP = 1;              //enable ADC scan complete interrupt
    ADC->INTEN_b.CNVCMP = 1;              //enable ADC conversion complete interrupt

    ADC->CFG = u32Cfg;
    
    ADC->INTCLR = 0xFFFFFFFF;             //clear interrupts
    ADC->SWT = 0x37;                      //trigger ADC



    UartInit(UART0,230400);                //init UART with 115200 baud

    GPIO->PADKEY = 0x00000073;            //unlock pin selection
    
    GPIO->PADREGF_b.PAD22FNCSEL = 0;      //set UARTTX on pin 22
    GPIO->PADREGF_b.PAD22INPEN = 1;       //enable input on pin 22
    GPIO->CFGC_b.GPIO22OUTCFG = 1;        //output is push-pull
    
    
    GPIO->PADKEY = 0;                     //lock pin selection
    

    while(1)
    {
				if (ADC->INTSTAT_b.CNVCMP == 1)
        {
            ADC->INTCLR_b.CNVCMP = 1;

        
            if ((_FLD2VAL(ADC_FIFO_COUNT,ADC->FIFO) > 0) && (_FLD2VAL(ADC_FIFO_SLOTNUM,ADC->FIFO) == 0))
            {
                u32Data = _FLD2VAL(ADC_FIFO_DATA,ADC->FIFO) >> 6;
								sprintf(data, "%d-",u32Data);
								PutStringUart(UART0,data);
                ADC->FIFO = 0; // pop FIFO
								
            }
        } 
				if (ADC->INTSTAT_b.SCNCMP == 1)
        {
            ADC->INTCLR_b.SCNCMP = 1;
            ADC->SWT = 0x37;
        }
        
    }
    return 0;
}

/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
