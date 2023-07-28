//project code :: 0607202325662348A

//import library
#include <PCF8563.h>
#include <Servo.h>
#include <Wire.h>                   // Library for I2C communication

//define value
#define VREF 3.3                    // analog reference voltage of the ADC
#define SCOUNT  30                  // sum of sample point

//define analog pin
#define TDSSENSOR_PIN_A0 A0         // tds sensor

//define digital pin
#define RELAY_PIN_2 2               // water pump number 2
#define RELAY_PIN_3 3               // water pump number 1
#define RELAY_PIN_4 4               // air pump number 2
#define RELAY_PIN_5 5               // air pump number 1
#define PUMP_PIN_6 6                // medicine pump
#define SERVO_PIN_7 7

//declare variable
PCF8563 pcf;
Servo servo;

int hour = 0;                       //set time
int analogBuffer[SCOUNT];           // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;           // Floats to calculate average voltage & TDS value
float tdsValue = 0;
float temperature = 25;             // current temperature for compensation

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup() {

  Serial.begin(115200);

  pinMode(TDSSENSOR_PIN_A0, INPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);
  pinMode(RELAY_PIN_5, OUTPUT);
  pinMode(PUMP_PIN_6, OUTPUT);

  servo.attach(7);

  pcf.init();                       //initialize the clock
  pcf.stopClock();                  //stop the clock
  pcf.setYear(23);                  //set year
  pcf.setMonth(7);                  //set month
  pcf.setDay(21);                   //set day
  pcf.setHour(2);                  //set hour
  pcf.setMinut(14);                 //set minute
  pcf.setSecond(0);                 //set second
  pcf.startClock();                 //start the clock

}

void loop() {

  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) { //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TDSSENSOR_PIN_A0);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0;

      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);

      //temperature compensation
      float compensationVoltage = averageVoltage / compensationCoefficient;

      //convert voltage value to tds value
      tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;

      if (tdsValue < 300) {
        Serial.print("Excellent TDS Value:");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");

        digitalWrite(RELAY_PIN_2, HIGH);
        digitalWrite(RELAY_PIN_3, LOW);
        digitalWrite(RELAY_PIN_4, HIGH);
        digitalWrite(RELAY_PIN_5, LOW);

      }
      else if (tdsValue < 600) {
        Serial.print("Good TDS Value:");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");

        digitalWrite(RELAY_PIN_2, HIGH);
        digitalWrite(RELAY_PIN_3, LOW);
        digitalWrite(RELAY_PIN_4, HIGH);
        digitalWrite(RELAY_PIN_5, LOW);

      }
      else if (tdsValue < 900) {
        Serial.print("Fair TDS Value:");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");

        digitalWrite(RELAY_PIN_2, HIGH);
        digitalWrite(RELAY_PIN_3, LOW);
        digitalWrite(RELAY_PIN_4, HIGH);
        digitalWrite(RELAY_PIN_5, LOW);

      }
      else if (tdsValue < 1000) {
        Serial.print("Poor TDS Value:");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");

        digitalWrite(RELAY_PIN_2, HIGH);
        digitalWrite(RELAY_PIN_3, HIGH);
        digitalWrite(RELAY_PIN_4, HIGH);
        digitalWrite(RELAY_PIN_5, HIGH);
      
      }
      else if (tdsValue == 1000) {
        Serial.print("Unacceptable TDS Value:");
        Serial.print(tdsValue, 0);
        Serial.println("ppm");
        
        digitalWrite(RELAY_PIN_2, HIGH);
        digitalWrite(RELAY_PIN_3, HIGH);
        digitalWrite(RELAY_PIN_4, HIGH);
        digitalWrite(RELAY_PIN_5, HIGH);
        
      }
      else {}
      
      Time nowTime = pcf.getTime();//get current time

      if (nowTime.day == 7) {
        if (nowTime.hour == 6) {
          if (nowTime.minute == 0) {
            if (nowTime.second == 0) {
              digitalWrite(PUMP_PIN_6, HIGH);
            }
            else{
              digitalWrite(PUMP_PIN_6, LOW);
            }
          }
          else {
            digitalWrite(PUMP_PIN_6, LOW);
          }
        }
        else {
          digitalWrite(PUMP_PIN_6, LOW);
        }
      }
      else if (nowTime.day == 14) {
        if (nowTime.hour == 6) {
          if (nowTime.minute == 0) {
            if (nowTime.second == 0) {
              digitalWrite(PUMP_PIN_6, HIGH);
            }
            else {
              digitalWrite(PUMP_PIN_6, LOW);
            }
          }
          else {
            digitalWrite(PUMP_PIN_6, LOW);
          }
        }
        else {
          digitalWrite(PUMP_PIN_6, LOW);
        }
      }   
      else if (nowTime.day == 21) {
        if (nowTime.hour == 6) {
          if (nowTime.minute == 0) {
            if (nowTime.second == 0) {
              digitalWrite(PUMP_PIN_6, HIGH);
            }
            else {
              digitalWrite(PUMP_PIN_6, LOW);
            }
          }
          else {
            digitalWrite(PUMP_PIN_6, LOW);
          }
        }
        else {
          digitalWrite(PUMP_PIN_6, LOW);
        }
      } 
      else if (nowTime.day == 28) {
        if (nowTime.hour == 6) {
          if (nowTime.minute == 0) {
            if (nowTime.second == 0) {
              digitalWrite(PUMP_PIN_6, HIGH);
            }
            else {
              digitalWrite(PUMP_PIN_6, LOW);
            }
          }
          else {
            digitalWrite(PUMP_PIN_6, LOW);
          }
        }
        else {
          digitalWrite(PUMP_PIN_6, LOW);
        }
      } 
      else {
       digitalWrite(PUMP_PIN_6, LOW);
      }
      
      if (nowTime.hour == 6) {
        if (nowTime.minute == 0) {
          if (nowTime.second == 0) {
            servo.write(0);
            delay(50);
            servo.write(40);
          }
          else {
            servo.write(40);
          }
        }
        else {
            servo.write(40);
        }
      }
      else if (nowTime.hour == 18) {
        if (nowTime.minute == 0) {
          if (nowTime.second == 0) {
            servo.write(0);
            delay(50);
            servo.write(40);
          }
          else {
            servo.write(40);
          }
        }
        else {
            servo.write(40);
        }
      }
      else {
       servo.write(40);
      }

      //print current time
      Serial.print(nowTime.day);
      Serial.print("/");
      Serial.print(nowTime.month);
      Serial.print("/");
      Serial.print(nowTime.year);
      Serial.print(" ");
      Serial.print(nowTime.hour);
      Serial.print(":");
      Serial.print(nowTime.minute);
      Serial.print(":");
      Serial.print(nowTime.second);
    }
  }
}
