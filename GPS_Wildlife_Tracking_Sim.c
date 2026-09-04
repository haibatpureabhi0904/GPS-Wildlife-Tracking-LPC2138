#include <lpc213x.h>
#include "lcd.h"
/* ==========================================
 * Geofence Boundaries (No Decimals!)
 * ========================================== */
// Boundaries mapped around Lat 3014.1984, Lon 09749.2872
#define MAX_LAT 30142500
#define MIN_LAT 30141500
#define MAX_LON 97493500
#define MIN_LON 97492000

/* ==========================================
 * Hardware Pin Definitions
 * ========================================== */
#define GREEN_LED (1 << 13) // P0.13
#define RED_LED   (1 << 14) // P0.14
#define BUZZER (1 << 15) // Buzzer connected to P0.15

// Global buffer for UART string collection
char rx_buffer[100];

/* ==========================================
 * Function Prototypes
 * ========================================== */
void uart0_init(void);
void uart1_init(void);
void uart0_tx(unsigned char data);
void uart0_send_string(char* str);
void get_gps_string(char *buffer);
void gps_data_parse(char* nmea_sentence);
void send_sms(char* lat, char* lon);
long coord_to_long(char* str);
void hardware_init(void);
void buzzer_on(void);
void buzzer_off(void);


/* ==========================================
 * Main Function
 * ========================================== */
int main()
{
    VPBDIV = 0x01; // PCLK = CCLK

    lcd_init();
    uart0_init();  // Initialize UART0 for GSM Module
    uart1_init();  // Initialize UART1 for GPS Module
    hardware_init(); // Initialize Buzzer and Leds

    while(1)
    {
        lcd_cmd(0x01);
        delay_ms(2);
        lcd_cmd(0x80);
        lcd_str((unsigned char*)"Tracking...     ");
        
        // 1. Block and wait for a full GPS string
        get_gps_string(rx_buffer); 
        
        // 2. Parse string, check boundaries, sound buzzer, and send SMS
        gps_data_parse(rx_buffer);
        
        // 3. Keep the status on screen for a few seconds before checking again
        delay_ms(4000);
    }
}

/* ==========================================
 * Hardware Control Functions (Buzzer)
 * ========================================== */
void hardware_init(void)
{
    // Set P0.13, P0.14, and P0.15 as Outputs
    IODIR0 |= (GREEN_LED | RED_LED | BUZZER); 
    
    // Ensure everything is OFF at startup
    IOCLR0 = (GREEN_LED | RED_LED | BUZZER);  
}

void buzzer_on(void)
{
    IOSET0 = BUZZER;  // Turn Buzzer ON
}

void buzzer_off(void)
{
    IOCLR0 = BUZZER;  // Turn Buzzer OFF
}

/* ==========================================
 * UART Data Acquisition (GPS / GSM)
 * ========================================== */
void uart1_init(void)
{
    PINSEL0 |= 0x00050000; // Set P0.8 as TXD1 and P0.9 as RXD1 (GPS)
    U1LCR = 0x83;          // 8-bit, no parity, 1 stop bit, DLAB=1
    
    // IMPORTANT: Baud rate configuration
    // Use 104 if Proteus LPC2138 Clock is 16MHz
    // Use 78 if Proteus LPC2138 Clock is 12MHz
    U1DLL = 78;           
    U1DLM = 0;
    U1LCR = 0x03;          // Lock baud rate 
}

void uart0_init(void)
{
    PINSEL0 |= 0x00000005; // Set P0.0 as TXD0 and P0.1 as RXD0 (GSM)
    U0LCR = 0x83;          
    U0DLL = 78;           // MUST MATCH U1DLL VALUE
    U0DLM = 0;
    U0LCR = 0x03;          
}

void uart0_tx(unsigned char data) 
{
    while (!(U0LSR & 0x20)); 
    U0THR = data;
}

void uart0_send_string(char* str)
{
    while(*str != '\0')
    {
        uart0_tx(*str);
        str++;
    }
}

void get_gps_string(char *buffer)
{
    char c;
    int i = 0;
    
    // 1. Wait until we receive the '$' symbol (Ignores watermarks)
    do 
    {
        while (!(U1LSR & 0x01)); 
        c = U1RBR;
    } 
    while (c != '$');
    
    buffer[i++] = c; 
    
    // 2. High-Speed Capture (No LCD commands allowed here)
    while (1)
    {
        while (!(U1LSR & 0x01)); 
        c = U1RBR;
        
        if (c == '\r' || c == '\n') 
        {
            buffer[i] = '\0'; // Null-terminate the string
            break;            
        }

        if (i < 99) 
        {
            buffer[i++] = c;
        }
    }
}

/* ==========================================
 * GSM Transmission Function
 * ========================================== */
void send_sms(char* lat, char* lon)
{
    // 1. Wake up GSM module
    uart0_send_string("AT\r\n");
    delay_ms(1000); 
    
    // 2. Set SMS to Text Mode
    uart0_send_string("AT+CMGF=1\r\n");
    delay_ms(1000);
    
    // 3. Set destination phone number
    uart0_send_string("AT+CMGS=\"+1234567890\"\r\n");
    delay_ms(1000);
    
    // 4. Construct message payload
    uart0_send_string("Alert! Animal outside safe zone.\r\nLat: ");
    uart0_send_string(lat);
    uart0_send_string("\r\nLon: ");
    uart0_send_string(lon);
    
    // 5. Send CTRL+Z (Hex 0x1A) to execute the transmission
    uart0_tx(0x1A);
    delay_ms(3000); 
}

/* ==========================================
 * Custom Math & Parser
 * ========================================== */
long coord_to_long(char* str) 
{
    long val = 0;
    int i = 0;
    while(str[i] != '\0') 
    {
        if (str[i] >= '0' && str[i] <= '9') 
        {
            val = (val * 10) + (str[i] - '0');
        }
        i++;
    }
    return val;
}

void gps_data_parse(char* nmea_sentence)
{
    int comma_count = 0;
    char latitude[15] = "";
    char longitude[15] = "";
    int i = 0, j = 0, k = 0;
    long current_lat = 0; 
    long current_lon = 0; 
    
    // 1. Manual string validation (Bypasses strncmp bugs)
    if (nmea_sentence[0] != '$' || nmea_sentence[1] != 'G' || 
        nmea_sentence[2] != 'P' || nmea_sentence[3] != 'R' || 
        nmea_sentence[4] != 'M' || nmea_sentence[5] != 'C')
    {
        return; 
    }
    
    // 2. Extract coordinates based on commas
    while(nmea_sentence[i] != '\0')
    {
        if(nmea_sentence[i] == ',')
        {
            comma_count++;
            i++;
            continue;
        }
        if(comma_count == 3)
        {
            latitude[j++] = nmea_sentence[i];
            latitude[j] = '\0';
        }
        if(comma_count == 5)
        {
            longitude[k++] = nmea_sentence[i];
            longitude[k] = '\0';
        }
        i++;
    }
    
    // 3. String to long integer conversion (Crash-proof math)
    current_lat = coord_to_long(latitude);
    current_lon = coord_to_long(longitude);
    
    // 4. Display Coordinates on top row
    lcd_cmd(0x01); 
    delay_ms(2);
    lcd_cmd(0x80); 
    lcd_str((unsigned char*)"Lat: ");
    lcd_str((unsigned char*)latitude);
    
    // 5. Geofence Check and Output
    lcd_cmd(0xC0); 
    if (current_lat > MAX_LAT || current_lat < MIN_LAT || 
        current_lon > MAX_LON || current_lon < MIN_LON)
    {
        // ANIMAL HAS ESCAPED!
        lcd_str((unsigned char*)"ALERT: OUTSIDE! ");
        IOSET0 = RED_LED;              // Turn RED LED ON
        IOCLR0 = GREEN_LED;            // Turn GREEN LED OFF
        IOSET0 = BUZZER;               // Turn Buzzer ON
        send_sms(latitude, longitude); // Text the ranger
    }
    else
    {
        // ANIMAL IS SAFE!
        lcd_str((unsigned char*)"Status: SAFE    ");
        IOSET0 = GREEN_LED;            // Turn GREEN LED ON
        IOCLR0 = RED_LED;              // Turn RED LED OFF
        IOCLR0 = BUZZER;               // Turn Buzzer OFF
    }
}
