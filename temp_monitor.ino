/*
 DHT22 Web Service
 
 Created by Matthew S. Cotterell on 15th Feburary 2012 (GPG Key ID: C9B4017475E2396C)
 Based off projects by "ladyada", David A. Mellis and Tom Igoe 
 
 Circuit:
 * Ethernet shield attached to pins D10, D11, D12, D13
 * DHT22 Humidity/Temperature sensor attached to pin D2
 * TEMT6000 Light Sensor attached to analog pin A0
 * 16x2 LCD Shield attached to pins D4, D5, D6, D7, D8 and D9
 * Optional SD Card for logging
 
 Third-Party Libraries:
 Modified version of the Freetronics DHT Sensor Library (https://github.com/freetronics/DHT-sensor-library/)
 
 */

#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22
#define CHIPSELECT 4

DHT dht(DHTPIN, DHTTYPE);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

//Pins for the freetronics 16x2 LCD shield.
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );

void setup()
{
  // start the Ethernet connection and the server:
  Ethernet.begin(mac);
  server.begin();
  dht.begin();
  SD.begin(CHIPSELECT);
  lcd.begin(16,2);
}

void loop()
{
  
  float h = dht.readHumidity(); // Humidity
  float t = dht.readTemperature(); // Temperature
  int l = analogRead(A0); // Light level
  
  // SD Card Logging
  boolean sderror = false;
  File dataFile = SD.open("arduino_data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(h);
    dataFile.print(",");
    dataFile.print(t);
    dataFile.print(",");
    dataFile.println(l);
    dataFile.close();
  }
  else
  {
    sderror = true;
  }
  
  // Liquid Crystal Display
  lcd.setCursor( 0, 0 );   //top left
  //         1234567890123456
  lcd.print("Humidity:   ");
  lcd.print(             h);
  //
  lcd.setCursor( 0, 1 );   //bottom left
  //         1234567890123456
  lcd.print("Temperature:");
  lcd.print(             t);
  
  // Ethernet HTTP Interface
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/xml");
          client.println();
          client.println("<?xml version=\"1.0\"?>");
           // check if returns are valid, if they are NaN (not a number) then something went wrong!
          if (isnan(t) || isnan(h)) {
            client.println("<error>DHTReadError</error>");
          } else {
            client.println("<dhtdata>");
            client.println("<humidity>");
            client.println(h);
            client.println("</humidity>");
            client.println("<temperature>");
            client.println(t);
            client.println("</temperature>");
            client.println("<lightlevel>");
            client.println(l);
            client.println("</lightlevel>");
            client.println("<sdlogging>");
            if (sderror) {
              client.println("false");
            }
            else
            {
              client.println("true");
            }
            client.println("</sdlogging>");
            client.println("</dhtdata>");
          }
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
