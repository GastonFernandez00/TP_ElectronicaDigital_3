Electrónica Digital 3
Trabajo Práctico Final




Integrantes:

Fernández, Gastón Emanuel - 42386463 -Ing. Computación
Oberto, Francisco - 42051961 - Ing. Computación

Profesores:
Sanchez, Julio  
Gallardo, Fernando

Repositorio:
GastonFernandez00/TP_ElectronicaDigital_3 (github.com)

Introducción
	A lo largo de este trabajo, se buscará dar una visión general del funcionamiento del microcontrolador LPC1769. Para ello se utilizará un sensor analógico de temperatura LM35 y el ADC integrado en el microcontrolador. Con éste, convertiremos la señal del sensor en una señal digital. 
Se utilizará el TIMER para controlar la frecuencia de conversión.
	Además, se utilizará el DAC para generar las señales analógicas de salida que harán sonar un buzzer en función del alejamiento del nivel de temperatura medido respecto de un valor de referencia.
	Por otro lado, se pasarán los valores de temperatura a otro modulo de UART para recibir los datos, y luego con GPIO configuramos el LCD para mostrar el dato por pantalla. 
	Por último, el pasaje de la temperatura, almacenada en memoria, hacia el UART será a través de DMA.
Objetivos
Implementar un proyecto integrador con todos los módulos vistos en clase sobre el microcontrolador LPC1769
Implementar ADC, DAC, TIMER, DMA y UART
Utilizar sensores y aplicaciones reales que puedan ser implementados en el ámbito profesional.

Diseño

En este trabajo se utilizará un sensor de temperatura LM35 en su modo de funcionamiento 
“Basic Centigrade Temperature Sensor”. Este modo de funcionamiento soporta temperaturas de entre 2°C y 150°C. A la salida se verán 0 volts en caso de medir el mínimo, 2°C. Además tiene un incremento de 10mV por cada °C que aumenta. Por lo que el voltaje de salida Vout se puede calcular de la siguiente manera.
Vout=(T-2°C)*10mV/°C
Siendo T la temperatura en °C medida por el sensor.





A continuación se puede ver un diagrama del sensor LM35.


Configuración del Timer

El control de la frecuencia de muestreo se logrará mediante la configuración precisa del TIMER0. Este se establecerá en modo MATCH, específicamente utilizando el Channel 1. Se implementarán interrupciones que se activarán con cada evento de coincidencia (match). Además, se reseteará el contador cada vez que se alcance este punto de coincidencia, creando así un bucle de operación continuo e infinito.

La frecuencia de conversión objetivo para esta configuración es de 4 Hz. Para lograr esto, el valor de coincidencia (match value) se calcula teniendo en cuenta la frecuencia base del timer y el objetivo de 4 ciclos por segundo. El valor de match se puede calcular de la siguiente manera.
M=(T-1)2

La configuración del Convertidor Analógico-Digital (ADC) se establece en relación directa con el comportamiento del temporizador (Timer) MAT0.1. Dado que el ADC ejecuta la conversión únicamente cuando MAT0.1 registra un flanco ascendente, se aplica una división por 2 en este contexto. Además, se opta por un valor de prescaler igual a cero para simplificar los cálculos involucrados.

Configuración del ADC

El ADC se configura para operar a una frecuencia de 12.5 MHz. Esta frecuencia de conversión es significativamente más alta que la del timer, asegurando así que no habrá problemas en la conversión de las muestras capturadas por el sensor. Se decide no emplear el modo burst y, en su lugar, iniciar la conversión con cada coincidencia en MATCH 0.1.

Configuración del DAC

En cuanto al Convertidor Digital-Analógico (DAC), se activa el modo 'bias'. Esta configuración limita la corriente máxima consumida por el DAC a 350µA y restringe su frecuencia de conversión a 400 KHz. Al igual que con el ADC, esta frecuencia supera ampliamente la del timer, garantizando así una conversión eficiente de las señales de salida correspondientes a cada nivel de temperatura detectado.

Transmisión Serial y Configuración de DMA

Para la transmisión serial a través del UART, se emplearán paquetes de datos de 8 bits. Esta configuración facilita la transmisión de las temperaturas medidas por el ADC hacia otro dispositivo, en este caso, el mismo microcontrolador, LPC1769, es quien recibe el dato. La configuración de la comunicación serial incluye dos bits de parada, sin bits de paridad, y una tasa de baudios configurada en 600. Además, se implementará una transmisión por DMA (Acceso Directo a Memoria), desde la memoria hasta la cola FIFO del UART.

En lo que respecta a la configuración del DMA, se optará por una transferencia de tipo memoria a periférico. Esta configuración funcionará en un bucle, transmitiendo continuamente los mismos datos de memoria hacia el UART3. El tamaño de cada transferencia será de 8 bits, acorde con la configuración de los paquetes de datos del UART.

Diseño y Funcionamiento del Manejador de Interrupciones del Timer

Incremento de la Variable de Control:

Al inicio de la función TIMER0_IRQHandler(), la variable j se incrementa. Esta variable actúa como un contador para controlar la frecuencia con la que el Timer ejecuta sus operaciones.




Lectura del Estado del Pin del Timer:

Se obtiene el estado actual del pin del TIMER mediante LPC_TIM0->EMR & 2, que se desplaza un bit a la derecha para aislar el estado relevante del pin.

Procesamiento en Flanco de Bajada:

Conversión de ADC: Si pinMatch es igual a cero (flanco de bajada), se lee la conversión analógica a digital (ADC) y se calcula el voltaje correspondiente.
Cálculo de Voltaje y Filtro de Ruido: Se convierte el valor del ADC a voltaje. Se aplica un filtro para eliminar el ruido, considerando solo los valores de voltaje entre 0.04 y 1.52 volts.
Cálculo de Temperatura: A partir del voltaje filtrado, se calcula la temperatura.
Determinación del Valor del DAC: Dependiendo de la temperatura calculada, se establece un valor para el DAC, ajustando la intensidad de la señal de salida en función de la temperatura detectada. El valor del voltaje a la salida del  dac se puede calcular de la siguiente manera.
V=value*3.3/1024

Procesamiento en Flanco de Subida:

Si se detecta un flanco de subida, y la temperatura está entre 27 y 45 grados Celsius, se actualiza el valor del DAC para generar una señal intermitente.

Actualización y Control de Periféricos:

Se actualiza el valor del DAC con DAC_UpdateValue().
Se reinicia el contador del DMA para permitir la transferencia continua de datos.
Reseteo y Limpieza de Interrupciones:

Al final de la función, se resetea el contador del Timer y se limpian las interrupciones pendientes, preparando el sistema para la siguiente interrupción.

Buzzer y amplificador de corriente

En esta sección se presenta un diagrama del circuito amplificador de corriente utilizado para hacer sonar el buzzer, con un divisor de tensión para poder hacer sonar el  buzzer a distintos valores de intensidad.
Funcionamiento

ADC (°C)

DAC


CIRCUITO
 
UART

Conclusión
Este trabajo final para la asignatura de Electrónica Digital 3 ha sido una oportunidad invaluable para aplicar y profundizar los conocimientos adquiridos en el campo de la microelectrónica. A través del diseño e implementación de un sistema basado en el microcontrolador LPC1769, se han integrado de manera efectiva distintos módulos - ADC, DAC, TIMER, DMA, y UART - para desarrollar una aplicación práctica y funcional.

La utilización del sensor de temperatura LM35 y la posterior conversión de sus señales analógicas en digitales mediante el ADC, ha permitido no solo comprender sino también aplicar conceptos clave de la electrónica digital. La configuración del TIMER para controlar la frecuencia de muestreo, así como el uso del DAC para generar señales de salida, ha demostrado ser esencial para el correcto funcionamiento del sistema.

Además, la implementación de la transmisión serial a través del UART y la gestión eficiente de la memoria mediante DMA, han sido aspectos fundamentales para asegurar la precisión y eficiencia en la comunicación de datos.

A través de este proyecto, se ha logrado no solo una comprensión más profunda de los principios de la electrónica digital, sino también la capacidad de aplicar estos principios en un contexto práctico y profesional. Las habilidades y conocimientos adquiridos durante este trabajo son transferibles a una amplia gama de aplicaciones en la industria y la investigación.

En conclusión, este trabajo no solo ha cumplido con los objetivos académicos establecidos, sino que también ha servido como un valioso ejercicio práctico en el campo de la electrónica digital y sus aplicaciones.


