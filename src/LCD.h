#ifndef LCD_H
#define LCD_H

void lcdConfigCompleta();
void lcdGpioConf();
void lcdDelay(uint16_t valor);
void lcdInit();
void ldcEnable();
void lcdDisable();
void ldcEnableDisable();
void lcdSET();
void lcdShow();
void lcdClear();
void lcdEntryMode();
void lcdShow2();
void lcdDataSend(uint8_t,uint32_t);
void escritura();
void noEscritura();
void lcdPinCtrl();


#endif
