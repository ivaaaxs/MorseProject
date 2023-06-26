#include <avr/io.h>
#include "AVR VUB/avrvub.h"
#include "Interrupt/interrupt.h"
#include "UART/uart.h"
#include "LCD/lcd.h"
#include "Timer/timer.h"
#include <util/delay.h>

volatile uint32_t impulse_nr = 0;
volatile bool ready_for_new_msg = true;
volatile bool flag_switch = false;

ISR(TIMER1_OVF_vect) {
	if (flag_switch == true) {
		TCNT1 = 0;
		lcd_print("end");
		uart_print("\r");
		ready_for_new_msg = true;
		timer1_interrupt_OVF_disable();	
		flag_switch = false;
	}
	flag_switch = true;
}

//outer interrupt
ISR(INT0_vect) {
	
	//rising edge
	if (get_pin(PIND, PD0) == 1) {
		//new message
		if (ready_for_new_msg == true) {
			TCNT1 = 0;
			flag_switch = false;
			timer1_interrupt_OVF_enable();
			ready_for_new_msg = false;
			lcd_clrscr();
		//sending message or finishing
		} else {
			impulse_nr = TCNT1;
			TCNT1 = 0;
			
			//pause
			if (impulse_nr >= 21000 && impulse_nr <= 25000) {
				lcd_print(";");
				uart_print(";");
				
			} else {
				
			}
		}
	} else { //falling edge
		impulse_nr = TCNT1;
		TCNT1 = 0;

		//a dot
		if (impulse_nr >= 7032 && impulse_nr <= 8594) {
			lcd_print(".");
			uart_print(".");
		//a line
		} else if (impulse_nr >= 14844 && impulse_nr <= 16407) {
			lcd_print("-");
			uart_print("-");
		} else {
			
		}
	}

}


void initialization() {
	
	//output ports
	DDRD |= (1 << PD1);
	DDRB |= (1 << PB6);
	DDRB |= (1 << PB4);
	
	//input port
	DDRD &= ~(1 << PD0);
	
	//initialization of LCD display
	lcd_init();
	
	//enabling and configuration of outer interrupts
	EIMSK |= (1 << INT0);
	EICRA |= (0 << ISC01) | (1 << ISC00); //both edges will generate interrupt INT0 on pin PD0
	
	//timer configuration
	timer1_set_normal_mode();
	timer1_set_prescaler(TIMER1_PRESCALER_1024);
	
	interrupt_enable();
	//initialization of serial communication
	uart_init(19200);
}

int main(void) {
    
	initialization();
	
	while(1) {
		
		//sending signal
		_delay_ms(3000);
		
		
		if (uart_read_all() == true) {
			
			for (int i = 0; uart_buffer[i] != '\0'; i++) {
				if (uart_buffer[i] == '.') {
					PORTD |= (1 << PD1);
					PORTB |= (1 << PB4);
					_delay_ms(500);
					PORTD &= ~(1 << PD1);
					PORTB &= ~(1 << PB4);
					_delay_ms(500);
					
				} else if (uart_buffer[i] == '-') {
					PORTD |= (1 << PD1);
					PORTB |= (1 << PB4);
					_delay_ms(1000);
					PORTD &= ~(1 << PD1);
					PORTB &= ~(1 << PB4);
					_delay_ms(500);
					
				} else if (uart_buffer[i] == ';') {
					_delay_ms(1000);
					
				}
				
			}
			
			PORTB |= (1 << PB6);
			_delay_ms(4100);
			PORTB &= ~(1 << PB6);
			
		}
	
	}
	
   return 0;

}
		