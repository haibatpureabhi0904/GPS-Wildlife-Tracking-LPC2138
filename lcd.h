#include <lpc214x.h>

/* ==========================================
 * LCD FUNCTIONS 
 * ========================================== */

void lcd_init(void);
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_str(unsigned char *p);
void delay(unsigned int);
void delay_ms(unsigned int);
void delay_us(unsigned int);


void lcd_init(void)
{
    IODIR1 |= 0x07FF0000; // Set P1.16 to P1.26 as output
    delay_ms(20);         // Give LCD time to power up
    
    lcd_cmd(0x38);        // 8-bit, 2 lines
    lcd_cmd(0x0C);        // Display ON, Cursor OFF
    lcd_cmd(0x01);        // Clear screen
    delay_ms(2);          // Clearing screen takes extra time!
    lcd_cmd(0x80);        // 1st row, 0th column
}

void lcd_cmd(unsigned char c)
{
    IOCLR1 = (0xFF << 16);  // Clear old data
    IOSET1 = (c << 16);     // Send new command
    
    IOCLR1 = (1 << 24);     // RS = 0 (Command)
    IOCLR1 = (1 << 25);     // RW = 0 (Write)
    
    IOSET1 = (1 << 26);     // EN = 1 (NOTE: Ensure your Proteus EN pin is connected to P1.26)
    delay_us(50);           // Very short delay for EN pulse
    IOCLR1 = (1 << 26);     // EN = 0
    
    delay_ms(2);            // Commands need a little time to process
}

void lcd_data(unsigned char d)
{
    IOCLR1 = (0xFF << 16);  // Clear old data
    IOSET1 = (d << 16);     // Send text character
    
    IOSET1 = (1 << 24);     // RS = 1 (Data)
    IOCLR1 = (1 << 25);     // RW = 0 (Write)
    
    IOSET1 = (1 << 26);     // EN = 1
    delay_us(50);           // Very short delay!
    IOCLR1 = (1 << 26);     // EN = 0
    
    delay_us(100);          // Only wait 100 microseconds, NOT 10 milliseconds. 
}

void lcd_str(unsigned char *p)
{
    while(*p != '\0')
    {
        lcd_data(*p);
        p++;
    }
}

/* ==========================================
 * DELAY FUNCTIONS
 * ========================================== */
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<1500; j++); // Approx 1ms at 16MHz
}

void delay_us(unsigned int us)
{
    unsigned int i, j;
    for(i=0; i<us; i++)
        for(j=0; j<2; j++);    // Approx 1us at 16MHz
}
