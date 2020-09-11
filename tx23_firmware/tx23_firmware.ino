#define F_CPU 8000000UL // 8MHz
/*
 * Firmware für den Windmesser TX23 auf Basis des ATtiny44A (1.0.1)
 * 
 * Unterstützt das Auslesen der Windgeschwindigkeit
 * Die Datenübertragung erfolgt via UART.
 * 
 * -----------------------------------------------------------------------------------
 * Diese Software steht unter folgender Lizenz:
 * 
 * MIT License
 *
 * Copyright (c) 2020 Florian Abeln
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <util/delay.h>
#include <SoftwareSerial.h>

#define MEASSUREMENT_TIME 1

uint8_t wind_cnt;

SoftwareSerial serial(3, 4); // RX und TX

ISR(INT0_vect) {
  wind_cnt++;
}

int main(void) {
  // Interrupt intialiseren
  MCUCR |= (1<<ISC01) | (1<<ISC00) ;
  GIMSK |= (1<<INT0) ;
  
  serial.begin(4800);

  while (true) {
    wind_cnt = 0;

    // Durchschnittlichen Wert aus MEASSUREMENT_TIME
    for (uint8_t i = 0; i < MEASSUREMENT_TIME; i++) { 
      sei();
      _delay_ms(1000);
      cli();
    }
    wind_cnt /= MEASSUREMENT_TIME;

    // Windgeschwindigkeit in km/h * 10 berechnen
    float wind_speed = (float)wind_cnt / (float)MEASSUREMENT_TIME * 2.4;

    serial.print(wind_speed);
    serial.print("#");

    //TODO: Auswertung von Fotodioden- / Fototransistoren o.a.
  }
}
