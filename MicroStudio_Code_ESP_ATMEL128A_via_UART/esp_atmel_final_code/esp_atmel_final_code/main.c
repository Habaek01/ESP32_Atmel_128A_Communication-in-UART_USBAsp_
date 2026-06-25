/*
 * ATmega128A_Receiver_Corrected_Pins.c
 * Target Hardware: ATmega128A (16 MHz External Crystal)
 * Function: Evaluates ESP32 commands to switch matched LIG1, LIG2, and LIG3 pairs.
 */ 

#define F_CPU 16000000UL 
#include <avr/io.h>
#include <string.h>

// --- VERIFIED SCHEMATIC PINS (image_c1a36f.png) ---

// LOOP 1 (LIG1)
#define LED1_PIN    PC1   // TCH1_LED1_LIG1
#define RELAY1_PIN  PF2   // TCH1_REL3_LIG1

// LOOP 2 (LIG2)
#define LED2_PIN    PC3   // TCH3_LED3_LIG2
#define RELAY2_PIN  PF1   // TCH3_REL2_LIG2

// LOOP 3 (LIG3)
#define LED3_PIN    PC4   // TCH4_LED4_LIG3
#define RELAY3_PIN  PF0   // TCH4_REL1_LIG3

#define BUFFER_SIZE 32

// Function Prototypes
void uart0_init(uint16_t ubrr_value);
char uart0_receive_char(void);
void uart0_receive_string(char *buffer, uint8_t max_length);

int main(void)
{
    // --- ?? UNLOCK PORTF JTAG PINS ?? ---
    // Required to allow PF4, PF5, and PF6 to act as relay outputs instead of debug pins
    MCUCSR = (1 << JTD);
    MCUCSR = (1 << JTD);

    // 103 = 9600 Baud for a 16 MHz Clock
    uart0_init(103); 

    // 1. Set the specific LED and Relay pins as OUTPUTS
    DDRC |= (1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN);
    DDRF |= (1 << RELAY1_PIN) | (1 << RELAY2_PIN) | (1 << RELAY3_PIN);

    // 2. Start with everything strictly OFF for safety
    PORTC &= ~((1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN));
    PORTF &= ~((1 << RELAY1_PIN) | (1 << RELAY2_PIN) | (1 << RELAY3_PIN));

    char rx_buffer[BUFFER_SIZE];

    while (1)
    {
        // 3. Block and wait for a complete ESP32 command ending in '\n'
        uart0_receive_string(rx_buffer, BUFFER_SIZE);

        // 4. Parse the command and switch the exact hardware pairs
        
        // --- LOOP 1 (LIG1) ---
        if (strstr(rx_buffer, "L1ON") != NULL) {
            PORTC |= (1 << LED1_PIN);    
            PORTF |= (1 << RELAY1_PIN);  
        }
        else if (strstr(rx_buffer, "L1OFF") != NULL) {
            PORTC &= ~(1 << LED1_PIN);   
            PORTF &= ~(1 << RELAY1_PIN); 
        }
        
        // --- LOOP 2 (LIG2) ---
        else if (strstr(rx_buffer, "L2ON") != NULL) {
            PORTC |= (1 << LED2_PIN);    
            PORTF |= (1 << RELAY2_PIN);  
        }
        else if (strstr(rx_buffer, "L2OFF") != NULL) {
            PORTC &= ~(1 << LED2_PIN);   
            PORTF &= ~(1 << RELAY2_PIN); 
        }
        
        // --- LOOP 3 (LIG3) ---
        else if (strstr(rx_buffer, "L3ON") != NULL) {
            PORTC |= (1 << LED3_PIN);    
            PORTF |= (1 << RELAY3_PIN);  
        }
        else if (strstr(rx_buffer, "L3OFF") != NULL) {
            PORTC &= ~(1 << LED3_PIN);   
            PORTF &= ~(1 << RELAY3_PIN); 
        }
    }
}

// --- UART 0 SETUP ---
void uart0_init(uint16_t ubrr_value) {
    UBRR0H = (uint8_t)(ubrr_value >> 8);
    UBRR0L = (uint8_t)ubrr_value;
    UCSR0B = (1 << RXEN0); // Enable Receiver hardware
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 Data Bits, 1 Stop Bit
}

// --- UART READ BYTE ---
char uart0_receive_char(void) {
    while (!(UCSR0A & (1 << RXC0))); // Wait until a byte arrives
    return UDR0;
}

// --- UART READ STRING ---
void uart0_receive_string(char *buffer, uint8_t max_length) {
    uint8_t index = 0;
    char c;
    
    while (index < (max_length - 1)) {
        c = uart0_receive_char();
        
        if (c == '\0') continue; // Ignore stray null bytes
        
        // Stop reading exactly when the ESP32 sends the '\n'
        if (c == '\n' || c == '\r') {
            if (index > 0) break; // Break out to evaluate the token
            continue;
        }
        buffer[index++] = c;
    }
    buffer[index] = '\0'; // Cap the string so strstr() can process it safely
}