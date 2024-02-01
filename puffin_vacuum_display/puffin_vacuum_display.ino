#include "OLED_Driver.h"
#include "GUI_paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "ImageData.h"

/////////////////////////////////////
/* SERIAL BIT */
#include <SoftwareSerial.h>
const int SSERIAL_RX_PIN = 5;  //Soft Serial Receive pin
const int SSERIAL_TX_PIN = 6;  //Soft Serial Transmit pin
const int SSERIAL_CTRL_PIN= 4;   //RS485 Direction control
const int LED_PIN = 13;
const int RS485_TRANSMIT = HIGH;
const int RS485_RECEIVE = LOW;

// Create Soft Serial Port object and define pins to use
SoftwareSerial RS485Serial(SSERIAL_RX_PIN, SSERIAL_TX_PIN); // RX, TX

int byteReceived;

int chars_sent = 0;
char get_status_command[7] = "#01IGS\r";
char get_value_command[6] = "#01RD\r";
int byteToSend;

String value_string;

int loop_counter = 0;
/////////////////////////////////////

UBYTE *BlackImage;
UWORD Imagesize;

void setup() {

  // Setup the screen //
  System_Init();
  OLED_2IN42_Init();
  Driver_Delay_ms(500); 
  OLED_2IN42_Clear(); 

  // 0. Create a nsew image cache
  Imagesize = ((OLED_2IN42_WIDTH%8==0)? (OLED_2IN42_WIDTH/8): (OLED_2IN42_WIDTH/8+1)) * OLED_2IN42_HEIGHT;
  if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) { 
      Serial.print("Failed to apply for black memory...\r\n");
      return -1;
  }
  Paint_NewImage(BlackImage, OLED_2IN42_WIDTH, OLED_2IN42_HEIGHT, 270, BLACK);

  // 1. Select Image
  Paint_SelectImage(BlackImage);
  Paint_Clear(BLACK);

  // Drawing on the image   
  // Paint_DrawString_EN(10, 12, "PUFFIN Vacuum", &Font12, WHITE, WHITE);
  // Paint_DrawString_EN(10, 32, "-----", &Font20, WHITE, WHITE);
  // Paint_DrawString_EN(90, 37, "Torr", &Font8, WHITE, WHITE);
  // OLED_2IN42_Display(BlackImage);
  Paint_DrawString_EN(10, 10, "PUFFIN Vacuum", &Font12, WHITE, WHITE);
  Paint_DrawString_EN(10, 30, "---------", &Font16, WHITE, WHITE);
  Paint_DrawString_EN(10, 44, "Torr", &Font8, WHITE, WHITE);
  OLED_2IN42_Display(BlackImage);
  // Driver_Delay_ms(2000);  
  // Paint_Clear(BLACK);

  // OLED_2IN42_Clear();


  /////////////////////////////////////
  /* SERIAL BIT */  
  Serial.begin(19200);           // Start the built-in serial port

  pinMode(LED_PIN, OUTPUT);     // Configure any output pins
  pinMode(SSERIAL_CTRL_PIN, OUTPUT);    
  
  digitalWrite(SSERIAL_CTRL_PIN, RS485_RECEIVE);  // Put RS485 in receive mode  
  
  RS485Serial.begin(19200);   // Start the RS485 soft serial port 

  delay(2000);
  /////////////////////////////////////
}

void loop() {
  if (chars_sent < 6) 
  {
    byteToSend = get_value_command[chars_sent];
    digitalWrite(SSERIAL_CTRL_PIN, RS485_TRANSMIT);
    RS485Serial.write(byteToSend);
    delay(1);
    digitalWrite(SSERIAL_CTRL_PIN, RS485_RECEIVE);
    chars_sent++;
  }
  
  if (RS485Serial.available())  // If we have data from the gauge to read...
  {
    byteReceived = RS485Serial.read();    // Read received byte
    
    // If we see a * character, we know the next 13 characters will be the response
    if ((char)byteReceived == '*') 
    {
      value_string = "";
      // Serial.write(byteReceived);
      int chars_read = 1;
      while (chars_read < 13) {
        if (RS485Serial.available()) {
          byteReceived = RS485Serial.read();

          // Skip the first 
          if (chars_read >= 4)
          {
            // Serial.write(byteReceived);
            value_string += (char)byteReceived;
          }
          chars_read++;
        } 
      }

      // Update the display
      // 1. Select Image
      Paint_SelectImage(BlackImage);
      Paint_Clear(BLACK);

      // Drawing on the image   
      Paint_DrawString_EN(10, 10, "PUFFIN Vacuum", &Font12, WHITE, WHITE);
      char display_chars[9];
      value_string.toCharArray(display_chars, 9);
      Paint_DrawString_EN(10, 30, display_chars, &Font16, WHITE, WHITE);
      Paint_DrawString_EN(10, 44, "Torr", &Font8, WHITE, WHITE);
      OLED_2IN42_Display(BlackImage);

      Serial.println(value_string);
    }
  }

  if (loop_counter == 500) // ~5 seconds!
  {
    chars_sent = 0;
    loop_counter = 0;
  }

  loop_counter++;
  delay(10);
}
