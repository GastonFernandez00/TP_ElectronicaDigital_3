/*
 *
 * Explicacion
 * https://www.youtube.com/watch?v=3X8rOw3Au-A&ab_channel=Opentronika
 *
 * */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#define D4 1<<0
#define D5 1<<1
#define D6 1<<2
#define D7 1<<3

#define RS 	1<<5
#define RW 	1<<6
#define E 	1<<4

void configCompleta();
void gpiocon();
void delay(uint16_t valor);
void lcdInit();
void enable();
void disable();
void enableDisable();
void lcdSET();
void lcdShow();
void lcdClear();
void lcdEntryMode();
void lcdShow2();
void datasend(uint8_t);
void escritura();
void noEscritura();
void pinclr();

uint8_t x = 1;
uint32_t y = 0;

int main(void) {
	configCompleta();


	while(1){
		if(y==1000000){
		datasend(x);
		x++;
		y=0;
		}
		y++;
	}
}





void datasend(uint8_t valor){
	noEscritura();
	lcdClear();

	uint8_t Unidad = 0, Decena = 0, Centena = 0;

	 Centena = valor / 100;
	 valor	 = valor % 100;
	 Decena = valor / 10;
	 Unidad = valor % 10;

	escritura();
	//Centena
	pinclr();
	LPC_GPIO2->FIOSET |= D4|D5;
	enableDisable();
	delay(100);
	pinclr();
	LPC_GPIO2->FIOSET |= Centena<<0;
	enableDisable();
	delay(100);

	//Decena
	pinclr();
	LPC_GPIO2->FIOSET |= D4|D5;
	enableDisable();
	delay(100);
	pinclr();
	LPC_GPIO2->FIOSET |= Decena<<0;
	enableDisable();
	delay(100);

	//Unidad
	pinclr();
	LPC_GPIO2->FIOSET |= D4|D5;
	enableDisable();
	delay(100);
	pinclr();
	LPC_GPIO2->FIOSET |= Unidad<<0;
	enableDisable(100);
	delay(100);


	//for(uint16_t i = 0; i<3; i++)delay(500);
	//noEscritura();
	//lcdClear();
}
void pinclr(){
	LPC_GPIO2->FIOCLR |= D4|D5|D6|D7;
}

void configCompleta(){
	gpiocon();
	lcdInit();
	lcdSET();
	lcdShow();
	lcdClear();
	lcdEntryMode();
	lcdShow2();
}

void escritura(){ //Permite escribir valores
	LPC_GPIO2->FIOSET |= RS;
}

void noEscritura(){ //Permite entrar al modo de configuracion, y otras funciones como el clear
	LPC_GPIO2->FIOCLR|= RS;
}

void lcdShow2(){
	LPC_GPIO2->FIOCLR |= D4|D5|D6|D7;
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D4|D5|D6|D7;
	enableDisable();
	delay(100);
}
void lcdEntryMode(){
	LPC_GPIO2->FIOCLR |= D4|D5|D6|D7;
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D5|D6;
	enableDisable();
	delay(100);


}
void lcdClear(){ //Limpia la pantalla
	LPC_GPIO2->FIOCLR |= D4|D5|D6|D7;
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D4;
	enableDisable();
	delay(100);
}

void lcdShow(){
	/*
	 * Conf 1: 0
	 * Conf 2: D7 = 1	 *
	 * */
	LPC_GPIO2->FIOCLR |= D4|D5|D6|D7;
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D7;
	enableDisable();
	delay(100);
}


void lcdSET(){
	/*
	 * Conf 1: D5 = 1
	 * Conf 2: D5 = 1
	 * Conf 3: D7 = 1 para 2 lineas logicas || D7 = 0 para 1 linea logica
	 * */
	LPC_GPIO2->FIOSET |= D5;
	LPC_GPIO2->FIOCLR |= D4;
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D5;
	LPC_GPIO2->FIOCLR |= D4;
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D7;
	LPC_GPIO2->FIOCLR |= D5;
	enableDisable();
	delay(100);
}


void lcdInit(){
/*
 * Se tiene que mandar3 veces el mismo comando de reinicio para que el
 * lcd empiece a grabar
 *
 * */
	//REINICIO:
	delay(2236);
	LPC_GPIO2->FIOSET |= D5|D4; //primer comando
	enableDisable();
	delay(707);
	LPC_GPIO2->FIOSET |= D5|D4; //segundo comando
	enableDisable();
	delay(100);
	LPC_GPIO2->FIOSET |= D5|D4; //tercer comando
	enableDisable();
	delay(100);
}

void enable(){
	LPC_GPIO2->FIOSET |= E; //Habilita
}
void disable(){
	LPC_GPIO2->FIOCLR |= E; //Deshabilita
}

void enableDisable(){		//Prende y apaga el enable, para grabar un dato o comando
	enable();
	delay(150);
	disable();

}

void gpiocon(){

	for(uint8_t i = 0;i<16;i++){ // Pines del 0 al 7 como GPIO2
		LPC_PINCON->PINSEL4 &= ~(1<<i);
	}
	LPC_GPIO2->FIOCLR |= 0X7F;	// Limpia pines
	LPC_GPIO2->FIODIR |= 0X7F;	// Pines 0 al 7 como salidas

}

void delay(uint16_t valor){	//Delay, valor random

	for(uint16_t i = 0; i<valor;i++)for(uint16_t j = 0; j<valor;j++);
}

