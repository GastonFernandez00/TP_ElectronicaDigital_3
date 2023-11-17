#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"
#include "LCD.h"
#endif

#include <cr_section_macros.h>

typedef enum{
	micro = 0,
	mili,
	cuarto_segundo,
	medio_segundo,
	segundo
}Tiempo_Type;

// VARIABLES #####################################################################
uint32_t j = 0, adcv = 0, dac = 0; //Globales de Control
float volts = 0, temperature = 0;
uint32_t Tiempo[5]={25,25000,6250000,12500000,25000000}; //Periodos [micro,mili,segundo/2,segundo]
uint8_t pinMatch,temperatureToInt = 0, controlTemp = 0, rangoTemp[4] = {27,45,45,70}, *PrangoTempMin, *PrangoTempMax, Rx_VALUE = 0;



GPDMA_LLI_Type LLI;

// FUNCIONES #####################################################################
void configADC();
void configPin();
void configTimer();
void configDAC();
void configUART();
void configDMA();

// MAIN ##########################################################################
int main(void) {
	lcdConfigCompleta();
	configPin();
	configADC();
	configDAC();
	configUART();
	configTimer();
	configDMA();
    while(1);
    return 0;
}


// CONFIGURACIONES ##############################################################

void configTimer(){
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_4);

	TIM_MATCHCFG_Type match;
	match.MatchChannel = 1; // Match Channel 1
	match.IntOnMatch = ENABLE; // Interrupt on match
	match.StopOnMatch = DISABLE; // Stop OFF
	match.ResetOnMatch = ENABLE; // Loop
	match.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE; // P1.29 Toggle
	match.MatchValue = (Tiempo[cuarto_segundo]-1)/2;
	TIM_ConfigMatch(LPC_TIM0, &match);

	LPC_TIM0->EMR |= 3<<6; // P1.29 Toggle
	LPC_TIM0->PR = 0; // Prescaler 0

	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); // Clear Flags
	NVIC_EnableIRQ(TIMER0_IRQn); // Interrupt Enable
	NVIC_SetPriority(TIMER0_IRQn, 1); //Baja un nivel la prioridad

	TIM_Cmd(LPC_TIM0, ENABLE);
	TIM_ResetCounter(LPC_TIM0); // Inicia el contador en 0
}

void configADC(){
	LPC_SC->PCONP 			|= 1<<12;		// Enciendo ADC
	LPC_SC->PCLKSEL0		|= 3<<24;		// f = 12.5MHz
	LPC_ADC->ADCR 			|= 1<<8;		// Divisor de frecuencia por 1
	LPC_ADC->ADCR 			|= 0<<16;		// Modo burst = FALSE
	LPC_ADC->ADCR			|= (4<<24);		// Start = 100 -> Start on MATCH 0.1
	// No interrumpiremos por ADC
	//LPC_ADC->ADINTEN		|= 1<<0;		// El canal 0 produce interrupciones
	LPC_ADC->ADINTEN		&= ~(1<<8);		// Interrupciones por los canales individuales (obligatorio aunque no usemos interrupciones)

	LPC_ADC->ADCR			|= 1<<21;		//PDN = 1, En 1 Habilita el ADC
	NVIC_EnableIRQ(ADC_IRQn);
}

void configDAC(){
	DAC_Init(LPC_DAC);
	/* Limita la corriente maxima que consume el DAC a 350uA
	 * Frecuencia de conversion p/ bias en True es de 400KHz */
	DAC_SetBias(LPC_DAC, 1);
}

void configUART(){

	UART_CFG_Type UART,UART0;
	UART_FIFO_CFG_Type FIFO,FIFO0;


	//UART3
	UART.Databits			= UART_DATABIT_8; // 8 bits por dato
	UART.Stopbits 			= UART_STOPBIT_1; // 1 bit de parada
	UART.Parity				= UART_PARITY_NONE; // Sin bit de paridad
	UART.Baud_rate			= 600; // Tasa de 9600 Baudios

	FIFO.FIFO_DMAMode		= ENABLE; // Habilita la transmisión por DMA a la lista FIFO
	FIFO.FIFO_Level			= UART_FIFO_TRGLEV0; // 1 byte de datos
	FIFO.FIFO_ResetTxBuf	= ENABLE; // Resetea la lista FIFO de transmisión
	FIFO.FIFO_ResetRxBuf	= DISABLE; // No usamos recepción

	//UART0

	UART0.Databits			= UART_DATABIT_8; // 8 bits por dato
	UART0.Stopbits 			= UART_STOPBIT_1; // 1 bit de parada
	UART0.Parity			= UART_PARITY_NONE; // Sin bit de paridad
	UART0.Baud_rate			= 600; // Tasa de 9600 Baudios

	FIFO0.FIFO_DMAMode		= DISABLE; // Habilita la transmisión por DMA a la lista FIFO
	FIFO0.FIFO_Level		= UART_FIFO_TRGLEV0; // 1 byte de datos
	FIFO0.FIFO_ResetTxBuf	= DISABLE; // Resetea la lista FIFO de transmisión
	FIFO0.FIFO_ResetRxBuf	= ENABLE; // No usamos recepción

	UART_Init(LPC_UART3, &UART);
	UART_FIFOConfig(LPC_UART3, &FIFO);

	UART_Init(LPC_UART0, &UART0);
	UART_FIFOConfig(LPC_UART0, &FIFO0);

	UART_TxCmd(LPC_UART3, ENABLE); // Habilita transmisión

}


void configDMA(){


	GPDMA_Channel_CFG_Type DMA;

	LLI.SrcAddr = (uint32_t) &temperatureToInt; // Source var in memory
	LLI.DstAddr = (uint32_t) &(LPC_UART3->THR); // Destination UART3 THR
	LLI.NextLLI = (uint32_t) &LLI; // Se repite la transmisión
	LLI.Control= 1 //sizeof(temperatureToInt)- bytes
								| (0<<18) 	//source width 8 bit
								| (0<<21) 	//dest. width 8 bit
								| (0<<26) 	//source increment
								| (0<<27)	//dest increment
								;
	/*
	 * sizeof(temperatureToInt)<<0			TransferSize = 8bits
	 * 000<<18 		SourceWidth			= 8bits
	 * 2<<21		DestinationWidth	= 32bits
	 * 00<<26 		Static Address Source & Destination
	*/

	GPDMA_Init();

	DMA.ChannelNum 		= 0; //Ch0
	DMA.TransferSize 	= 1;//sizeof(temperatureToInt);
	DMA.TransferWidth 	= 0;
	DMA.SrcMemAddr 		= (uint32_t) &temperatureToInt;
	DMA.DstMemAddr		= 0;
	DMA.TransferType 	= GPDMA_TRANSFERTYPE_M2P;
	DMA.SrcConn 		= 0;
	DMA.DstConn			= (uint32_t) GPDMA_CONN_UART3_Tx;
	DMA.DMALLI			= (uint32_t) &LLI;


	GPDMA_Setup(&DMA);
	GPDMA_ChannelCmd(0, ENABLE);


}

void configPin(){
	PINSEL_CFG_Type pin;

/*
	CONFIG PIN0.0 Y PIN0.1
	PIN[0] -> Pin de entrada a interrupcion por GPIO
	PIN[1] -> Pin del led, ON = Umbral Alto, OFF = Umbral Bajo
*/
	LPC_GPIO0->FIODIR |= 2<<0; 		// 10 -> PIN[0] = ENTRADA PIN[1] = SALIDA
	LPC_GPIOINT->IO0IntEnR |= 1<<0; // Interrupcion por Rising en PIN[0]
	LPC_GPIO0->FIOCLR |= 1<<1;		//Limpia el PIN[1]
	LPC_PINCON->PINMODE0 |= 3<<0 | 3<<2;



	// Sensor de Temperatura LM35 - ADC00
	pin.Portnum = 0;
	pin.Pinnum = 23;
	pin.Funcnum = 1;
	pin.Pinmode = PINSEL_PINMODE_PULLDOWN;

	PINSEL_ConfigPin(&pin);

	// Match Pin MATCH 0 .1
	pin.Portnum = 1;
	pin.Pinnum = 29;
	pin.Funcnum = 3;

	PINSEL_ConfigPin(&pin);

	// Output DAC - DAC Pin P0.26
	pin.Portnum = 0;
	pin.Pinnum = 26;
	pin.Funcnum = 2;

	PINSEL_ConfigPin(&pin);

	// UART3 TX - Pin P0.25
	pin.Portnum = 0;
	pin.Pinnum = 25;
	pin.Funcnum = 3;

	PINSEL_ConfigPin(&pin);

	// UART0 RX - Pin P0.3
	pin.Portnum = 0;
	pin.Pinnum = 3;
	pin.Funcnum = 1;

	PINSEL_ConfigPin(&pin);

	PrangoTempMin = &rangoTemp[0];		//Establece los valores Iniciales de Temperatura
	PrangoTempMax = &rangoTemp[1];

	NVIC_SetPriority(EINT3_IRQn, 0);	//Prioridad de interrupcion en maximo nivel
	NVIC_EnableIRQ(EINT3_IRQn);			//Habilita las interrupciones GPIO
}

// INTERRUPCIONES ################################################################
void EINT3_IRQHandler(){
	switch(controlTemp){
	case 0: ////Rango Superior

		PrangoTempMin = &rangoTemp[2];	//35
		PrangoTempMax = &rangoTemp[3];	//70

		LPC_GPIO0->FIOSET |= 1<<1;		//Prende el LED, indicado rango de temperatura superior
		controlTemp = 1;
		break;

	case 1: //Rango Inferior

		PrangoTempMin = &rangoTemp[0];	//27
		PrangoTempMax = &rangoTemp[1];	//45

		LPC_GPIO0->FIOCLR |= 1<<1;		//Apaga el LED, indicado rango de temperatura inferior
		controlTemp = 0;
		break;

	}

	LPC_GPIOINT->IO0IntClr |= 1<<0; // Limpia la bander de interrupcion
}
void TIMER0_IRQHandler(){
	j++; // Variable de control de frecuencia del timer

	pinMatch = (LPC_TIM0->EMR & 2) >> 1; // Guardo valor del pin del TIMER

	// Flanco de bajada
	if (pinMatch == 0){
		adcv = (LPC_ADC->ADDR0 & 4095<<4)>>4; // Conversión del ADC

		volts = 3.3 * adcv / 4095; // Cálculo del voltaje leído por el ADC

		if (volts < 1.52 && volts > 0.04){ // Filtro de ruido
			temperature = volts / 0.01 - 2; // Temperatura sensada
		}


		if (temperature < *PrangoTempMin) {
			dac = 0; // No suena debajo de los 27°C
		} else if(temperature < *PrangoTempMax) {
			dac = 600; // Suena debilmente entre 27 y 45 °C
		}else{
			dac = 1023; // Suena fuertemente por encima de los 45°C
		}

		temperatureToInt = temperature; // Convierte temperatura float a uint8_t

		LLI.Control |= 1<<0; // Resetea contador del DMA

		//UART_SendByte(LPC_UART3, temperatureToInt);
		DAC_UpdateValue(LPC_DAC, dac); // Conversion del DAC

		lcdDataSend(UART_ReceiveByte(LPC_UART0)); //Actualiza Pantalla LCD


	}else{ // Flanco de subida
		if (temperature >  *PrangoTempMin && temperature < *PrangoTempMax){
			DAC_UpdateValue(LPC_DAC, 0); // Suena intermitentemente entre 27 y 45 °C
		}
		//temperatureToInt = 0xFF;
	}


	TIM_ResetCounter(LPC_TIM0); // Resetea el contador
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); // Limpia interrupciones del timer
}
