#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>


#define BAUD_RATE 9600
#define MYUBRR ((F_CPU / 16 / BAUD_RATE) - 1) // This section help us to know the what value should be used to get the require BAUD rate. 

#define NODE_ID     1
#define PIR_PIN     PD2
#define DHT_PIN     PD3
#define FAN_PIN     PB1
#define LED_PIN     PB5


volatile uint8_t pir_state = 0; // setting pir_state to 0 initially 
volatile uint8_t sample_flag = 0; 
volatile uint32_t timestamp = 0; // set time stamp to 0 initially 

uint8_t fan_state = 0;

void uart_init(void) {
    UBRR0H = (MYUBRR >> 8); // when the data is more than 8 bits we need to used both register to transfer such as UBRR0H and UBRR0L where 0L take first 8 bit of data 
    UBRR0L = MYUBRR;
    UCSR0B = (1 << TXEN0); // this section is used to set the UART communication enabling TX and RX and 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);// 8bit data 
}

void uart_send_char(char c) {
    while (!(UCSR0A & (1 << UDRE0))); // this register is used to send the data via UART 
    UDR0 = c;
}

void uart_send_string(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_uint(uint16_t n) {
    char buf[6];
    itoa(n, buf, 10);
    uart_send_string(buf);
}


ISR(TIMER1_COMPA_vect) { 
    sample_flag = 1;
    timestamp += 3;
}

ISR(INT0_vect) {
    pir_state = (PIND & (1 << PIR_PIN)) ? 1 : 0; // external interrupt set for pin 2 
}

void timer1_init(void) {
    TCCR1B |= (1 << WGM12);         // CTC mode was selected 
    OCR1A = 46874;       // OCR1A is 16 bits register thus it can handle timer less than 6 second.                
    TCCR1B |= (1 << CS12) | (1 << CS10); // we divide each tick by 16MHz then pre-scale with 1024 and assume every 1024 clock cycle is 1 tick so there will pre-scaled 15,625 clock cycle completed in 1 second.
    TIMSK1 |= (1 << OCIE1A);
}


uint8_t read_dht11(uint8_t *temp) { // This section is for DHT11 setup
    uint8_t data[5] = {0}; // we set the 5-bit array to store the sensor reponse 
 
    DDRD |= (1 << DHT_PIN); // initially sensor was set as output
    PORTD &= ~(1 << DHT_PIN); // pull the pin low by making it zero 
    _delay_ms(18);// delay for 18 millisecond 
    PORTD |= (1 << DHT_PIN); // pull the pin high 
    _delay_us(40); // delay for 40 millisecond
    DDRD &= ~(1 << DHT_PIN); // change the same pin as input 

    _delay_us(160);

    for (uint8_t i = 0; i < 5; i++) { // the dht 11 temperature, humidity with interger and float value 
        for (uint8_t j = 0; j < 8; j++) { // each bit contain 8 bit 
            while (!(PIND & (1 << DHT_PIN))); // the micro controller check if the dht pin is in HIGH state or low state and act according to it 
            uint8_t cnt = 0;
            while ((PIND & (1 << DHT_PIN)) && cnt < 100) { // if the state is high it measure the pulse time if the time is less than 30 milli second then the date is 0 bit else 1
                cnt++;
                _delay_us(1);
            }
            if (cnt > 30)
                data[i] |= (1 << (7 - j));
        }
    }

    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        *temp = data[2];
        return 1;
    }
    return 0;
}

void send_json(uint8_t temp) { // creating a json format
    uart_send_string("{\"node\":");
    uart_send_uint(NODE_ID);
    uart_send_string(",\"ts\":");
    uart_send_uint(timestamp);
    uart_send_string(",\"temp\":");
    uart_send_uint(temp);
    uart_send_string(",\"pir\":");
    uart_send_uint(pir_state);
    uart_send_string(",\"fan\":");
    uart_send_uint(fan_state);
    uart_send_string("}\r\n");
}

int main(void) {

    DDRD &= ~((1 << PIR_PIN) | (1 << DHT_PIN)); // PIR and DHT11 as input from pin 2 and pin3 of port D
    DDRB |= (1 << FAN_PIN) | (1 << LED_PIN); // Fan and led as output 

    PORTD |= (1 << PIR_PIN);  // PIR pull-up

    uart_init();

    
    EICRA |= (1 << ISC00);     // enable ISC01 as 0 and ISC00 as 1 will enable to trigger at any logic changes 
    EIMSK |= (1 << INT0); // enable INT0 Interrupt.

    timer1_init();
    sei();

    set_sleep_mode(SLEEP_MODE_IDLE); // sleep mode 

    uint8_t temperature = 0;

    while (1) {
        /
        if (sample_flag) {
            sample_flag = 0;

            if (read_dht11(&temperature)) {
                send_json(temperature);
            }
        }

        // This part is mainly responsible for the data receive from raspberry pie and accordingly where RXC0 register for receiving the bites from edge device.
        if (UCSR0A & (1 << RXC0)) {
            char cmd = UDR0;
            if (cmd == 'F') { // Fan ON
                PORTB |= (1 << FAN_PIN) | (1 << LED_PIN); // set FAN_PIN and LED_PIN high 
                fan_state = 1;
                uart_send_string("{\"status\":\"FAN ON\"}\r\n");
            } else if (cmd == 'O') { // Fan OFF
                PORTB &= ~((1 << FAN_PIN) | (1 << LED_PIN)); // set FAN_PIN and LED_PIN low 
                fan_state = 0;
                uart_send_string("{\"status\":\"FAN OFF\"}\r\n");
            } else if (cmd == 'S') { // Send current status
                send_json(temperature);
            }
        }

      
        sleep_cpu(); // sleep mode until next interrupt occur to save the power consumption 
    }
}