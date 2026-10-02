#include <Wire.h>
#include "font6x8.h"
// #include "font16x24.h"

// #include "img0_128x64c1.h"
// #include "digistump_128x64c1.h"

// uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

unsigned int oldPPM = 0;

// ----------------------------------------------------------------------------
// void ssd1306_init(void);
// void ssd1306_xfer_start(void);
// void ssd1306_xfer_stop(void);
// void ssd1306_send_byte(uint8_t byte);
// void ssd1306_send_command(uint8_t command);
// void ssd1306_send_data_start(void);
// void ssd1306_send_data_stop(void);
// void ssd1306_setpos(uint8_t x, uint8_t y);
// void ssd1306_fillscreen(uint8_t fill_Data);
// void ssd1306_char_f6x8(uint8_t x, uint8_t y, const char ch[]);
uint8_t INA = 4;

char* dtoa(double dN, char *cMJA, int iP) {
  char *ret = cMJA; long lP=1; byte bW=iP;
  while (bW>0) { lP=lP*10;  bW--;  }
  long lL = long(dN); double dD=(dN-double(lL))* double(lP); 
  if (dN>=0) { dD=(dD + 0.5);  } else { dD=(dD-0.5); }
  long lR=abs(long(dD));  lL=abs(lL);  
  if (lR==lP) { lL=lL+1;  lR=0;  }
  if ((dN<0) & ((lR+lL)>0)) { *cMJA++ = '-';  } 
  ltoa(lL, cMJA, 10);
  if (iP>0) { while (*cMJA != '\0') { cMJA++; } *cMJA++ = '.'; lP=10; 
  while (iP>1) { if (lR< lP) { *cMJA='0'; cMJA++; } lP=lP*10;  iP--; }
  ltoa(lR, cMJA, 10); }
  return ret;
}

void setup() {
  ssd1306_init();
  ssd1306_fillscreen(0x00);
  // ssd1306_char_f6x8(8, 2, "000");
  delay(3000);
  // ssd1306_fillscreen(0x00);

  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
}

void loop() {
  
  // ssd1306_char_f6x8(0, 0, "V");

  
  double reada = analogRead(2);
  // double readb = analogRead(3);
  // double readc = analogRead(0);

  // ssd1306_fillscreen(0x00);
  // ssd1306_char_f6x8(40, 1, "V");
  // String(x).toCharArray(b,2);
  // ssd1306_char_f16x24(20, 0, b);
  float v = float(map(reada, 0, 137, 0, 100)) / 10.0;
  char a[String(v).length() + 1];
  // char b[String(readb).length() + 1];
  // char c[String(readc).length() + 1];

  // itoa(v, a, 10);
  // itoa(readb, b, 10);
  // itoa(readc, c, 10);

  // ssd1306_char_f6x8(0, 1, a);
  // ssd1306_char_f6x8(0, 2, b);
  // ssd1306_char_f6x8(0, 3, c);

  ssd1306_char_f6x8(0, 1, dtoa(v, a, 1));
  // ssd1306_char_f6x8(0, 2, dtoa(readb, b, 1));
  // ssd1306_char_f6x8(0, 3, dtoa(readc, c, 1));

  delay(300);
}

// ================================================= //

#define DIGITAL_WRITE_HIGH(PORT) PORTB |= (1 << PORT)
#define DIGITAL_WRITE_LOW(PORT) PORTB &= ~(1 << PORT)

// Some code based on "IIC_wtihout_ACK" by http://www.14blog.com/archives/1358
#ifndef SSD1306XLED_H
#define SSD1306XLED_H
// ---------------------	// Vcc,	Pin 1 on SSD1306 Board
// ---------------------	// GND,	Pin 2 on SSD1306 Board
#ifndef SSD1306_SCL
#define SSD1306_SCL		PB2	// SCL,	Pin 3 on SSD1306 Board
#endif
#ifndef SSD1306_SDA
#define SSD1306_SDA		PB0	// SDA,	Pin 4 on SSD1306 Board
#endif
#ifndef SSD1306_SA
#define SSD1306_SA		0x78	// Slave address
#endif

// ----------------------------------------------------------------------------
#endif
void ssd1306_init(void) {


  DDRB |= (1 << SSD1306_SDA); // Set port as output
  DDRB |= (1 << SSD1306_SCL); // Set port as output

  ssd1306_send_command(0xAE); // display off
  ssd1306_send_command(0x00); // Set Memory Addressing Mode
  ssd1306_send_command(0x10); // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  ssd1306_send_command(0x40); // Set Page Start Address for Page Addressing Mode,0-7
  ssd1306_send_command(0x81); // Set COM Output Scan Direction
  ssd1306_send_command(0xCF); // ---set low column address
  ssd1306_send_command(0xA1); // ---set high column address
  ssd1306_send_command(0xC0); // --set start line address. Flip vertical C0 or C8
  ssd1306_send_command(0xA6); // --set contrast control register
  ssd1306_send_command(0xA0); // Flip horizontal A0 or A8
  ssd1306_send_command(0x1F); // --set segment re-map 0 to 127; 3F - 64; 1F - 32
  ssd1306_send_command(0xD3); // --set normal display
  ssd1306_send_command(0x00); // --set multiplex ratio(1 to 64)
  ssd1306_send_command(0xD5); //
  ssd1306_send_command(0x80); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  ssd1306_send_command(0xD9); // -set display offset
  ssd1306_send_command(0xF1); // -not offset
  ssd1306_send_command(0xDA); // --set display clock divide ratio/oscillator frequency
  ssd1306_send_command(0x02); // --set divide ratio
  ssd1306_send_command(0xDB); // --set pre-charge period
  ssd1306_send_command(0x40); //
  ssd1306_send_command(0x20); // --set com pins hardware configuration
  ssd1306_send_command(0x02);
  ssd1306_send_command(0x8D); // --set vcomh
  ssd1306_send_command(0x14); // 0x20,0.77xVcc
  ssd1306_send_command(0xA4); // --set DC-DC enable
  ssd1306_send_command(0xA6); // Set display not inverted
  ssd1306_send_command(0xAF); // --turn on oled panel
}

void ssd1306_xfer_start(void) {
  DIGITAL_WRITE_HIGH(SSD1306_SCL);	// Set to HIGH
  DIGITAL_WRITE_HIGH(SSD1306_SDA);	// Set to HIGH
  DIGITAL_WRITE_LOW(SSD1306_SDA);		// Set to LOW
  DIGITAL_WRITE_LOW(SSD1306_SCL);		// Set to LOW
}

void ssd1306_xfer_stop(void) {
  DIGITAL_WRITE_LOW(SSD1306_SCL);		// Set to LOW
  DIGITAL_WRITE_LOW(SSD1306_SDA);		// Set to LOW
  DIGITAL_WRITE_HIGH(SSD1306_SCL);	// Set to HIGH
  DIGITAL_WRITE_HIGH(SSD1306_SDA);	// Set to HIGH
}

void ssd1306_send_byte(uint8_t byte) {
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if ((byte << i) & 0x80)
      DIGITAL_WRITE_HIGH(SSD1306_SDA);
    else
      DIGITAL_WRITE_LOW(SSD1306_SDA);

    DIGITAL_WRITE_HIGH(SSD1306_SCL);
    DIGITAL_WRITE_LOW(SSD1306_SCL);
  }
  DIGITAL_WRITE_HIGH(SSD1306_SDA);
  DIGITAL_WRITE_HIGH(SSD1306_SCL);
  DIGITAL_WRITE_LOW(SSD1306_SCL);
}

void ssd1306_send_command(uint8_t command) {
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_SA);  // Slave address, SA0=0
  ssd1306_send_byte(0x00);	// write command
  ssd1306_send_byte(command);
  ssd1306_xfer_stop();
}

void ssd1306_send_data_start(void) {
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_SA);
  ssd1306_send_byte(0x40);	//write data
}

void ssd1306_send_data_stop(void) {
  ssd1306_xfer_stop();
}

void ssd1306_setpos(uint8_t x, uint8_t y)
{
  ssd1306_xfer_start();
  ssd1306_send_byte(SSD1306_SA);  //Slave address,SA0=0
  ssd1306_send_byte(0x00);	//write command

  ssd1306_send_byte(0xb0 + y);
  ssd1306_send_byte(((x & 0xf0) >> 4) | 0x10); // |0x10
  ssd1306_send_byte((x & 0x0f) | 0x01); // |0x01

  ssd1306_xfer_stop();
}

void ssd1306_fillscreen(uint8_t fill_Data) {
  uint8_t m, n;
  for (m = 0; m < 4; m++)
  {
    ssd1306_send_command(0xb0 + m);	//page0-page1
    ssd1306_send_command(0x00);		//low column start address
    ssd1306_send_command(0x10);		//high column start address
    ssd1306_send_data_start();
    for (n = 0; n < 128; n++)
    {
      ssd1306_send_byte(fill_Data);
    }
    ssd1306_send_data_stop();
  }
}


void ssd1306_char_f6x8(uint8_t x, uint8_t y, const char ch[]) {
  uint8_t c, i, j = 0;
  while (ch[j] != '\0')
  {
    c = ch[j] - 32;
    if (x > 126)
    {
      x = 0;
      y++;
    }
    ssd1306_setpos(x, y);
    ssd1306_send_data_start();
    for (i = 0; i < 6; i++)
    {
      ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font6x8[c * 6 + i]));
    }
    ssd1306_send_data_stop();
    x += 6;
    j++;
  }
}

// void ssd1306_char_f16x24(uint8_t tx, uint8_t ty, const char ch[]) {
//   int c, j = 0;
//   while (ch[j] != '\0')
//   {
//     c = ch[j] - 47;

//     uint16_t jj = c * 48;
//     uint16_t y, x;
//     uint16_t y1 = ty + 3;
//     uint16_t x1 = tx + 16;

//     for (y = ty; y < y1; y++)
//     {
//       ssd1306_setpos(tx, y);
//       ssd1306_send_data_start();
//       for (x = tx; x < x1; x++)
//       {
//         if (c >= 0) {
//           ssd1306_send_byte(pgm_read_byte(&ssd1306xled_font16x24[jj++]));
//         } else {
//           ssd1306_send_byte(0x00);
//         }
//       }
//       ssd1306_send_data_stop();
//     }

//     tx += 16;
//     j++;
//   }
// }

// void draw_bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[])
// {
//   uint16_t j = 0;
//   uint8_t y, x;
//   // if (y1 % 8 == 0) y = y1 / 8;   // else y = y1 / 8 + 1;   // tBUG :: this does nothing as y is initialized below
//   //  THIS PARAM rule on y makes any adjustment here WRONG   //usage oled.bitmap(START X IN PIXELS, START Y IN ROWS OF 8 PIXELS, END X IN PIXELS, END Y IN ROWS OF 8 PIXELS, IMAGE ARRAY);
//   for (y = y0; y < y1; y++)
//   {
//     ssd1306_setpos(x0, y);
//     ssd1306_send_data_start();
//     for (x = x0; x < x1; x++)
//     {
//       ssd1306_send_byte(pgm_read_byte(&bitmap[j++]));
//     }
//     ssd1306_send_data_stop();
//   }
// }
