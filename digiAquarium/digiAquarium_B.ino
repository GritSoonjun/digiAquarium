//project code :: 0607202325662348B

#include <Adafruit_Microbit.h>
#include <PCF8563.h>
#include <Wire.h>
#include <Servo.h>

//define pin
#define TDS 0
#define RELAY 1
#define PUMP 2

//tds value zone
#define VREF 3.3              // analog reference voltage of the ADC
#define SCOUNT  30            // sum of sample point

PCF8563 pcf;
Servo servo;
Adafruit_Microbit_Matrix microbit;

//set tds sensor
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;     // Floats to calculate average voltage & TDS value
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

//timeValue
int pTime = 100;
int sTime = 100;

const uint8_t
  smile_bmp[] =
  { B00111,
    B00101,
    B11111,
    B10101,
    B11101, };

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

  pinMode(TDS, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(PUMP, OUTPUT);

  pcf.init();//initialize the clock

  pcf.stopClock();//stop the clock

  pcf.setYear(23);//set year
  pcf.setMonth(1);//set month
  pcf.setDay(1);//set day
  pcf.setHour(2);//set hour
  pcf.setMinut(0);//set minute
  pcf.setSecond(0);//set second

  pcf.startClock();//start the clock

  servo.attach(8);

  microbit.begin();

}

void loop() {

  microbit.show(smile_bmp);

  Tds();
  Pump();
  Servo();
  
}

void Tds() {

  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) { //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TDS);    //read the analog value and store into the buffer
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

          Time nowTime = pcf.getTime();//get current time

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
          Serial.println(nowTime.second);

          digitalWrite(RELAY, LOW);
        }
        else if (tdsValue < 600) {
          Serial.print("Good TDS Value:");
          Serial.print(tdsValue, 0);
          Serial.println("ppm");

          Time nowTime = pcf.getTime();//get current time

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
          Serial.println(nowTime.second);
                  
          digitalWrite(RELAY, LOW);
        }
        else if (tdsValue < 900) {
          Serial.print("Fair TDS Value:");
          Serial.print(tdsValue, 0);
          Serial.println("ppm");

          Time nowTime = pcf.getTime();//get current time

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
          Serial.println(nowTime.second);

          digitalWrite(RELAY, LOW);
        }
        else if (tdsValue < 1000) {
          Serial.print("Poor TDS Value:");
          Serial.print(tdsValue, 0);
          Serial.println("ppm");

          Time nowTime = pcf.getTime();//get current time

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
          Serial.println(nowTime.second);

          digitalWrite(RELAY, HIGH);
        }
        else if (tdsValue == 1000) {
          Serial.print("Unacceptable TDS Value:");
          Serial.print(tdsValue, 0);
          Serial.println("ppm");

          Time nowTime = pcf.getTime();//get current time

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
          Serial.println(nowTime.second);
        
          digitalWrite(RELAY, HIGH);
        }
        else {}
      }
    }
}

void Pump() {

  Time nowTime = pcf.getTime();//get current time

  if (nowTime.hour == 6) {
    if (nowTime.minute == 0) {
      if (nowTime.second == 0) {
        if (nowTime.day == 1) {
          digitalWrite(PUMP, HIGH);
          delay(pTime);
          digitalWrite(PUMP, LOW);
        }
        else if (nowTime.day == 7) {
          digitalWrite(PUMP, HIGH);
          delay(pTime);
          digitalWrite(PUMP, LOW);
        }
        else if (nowTime.day == 14) {
          digitalWrite(PUMP, HIGH);
          delay(pTime);
          digitalWrite(PUMP, LOW);
        }
        else if (nowTime.day == 21) {
          digitalWrite(PUMP, HIGH);
          delay(pTime);
          digitalWrite(PUMP, LOW);
        }
        else {
          digitalWrite(PUMP, LOW);
        }
      }
      else {
        digitalWrite(PUMP, LOW);
      }
    }
    else {
      digitalWrite(PUMP, LOW);
    }
  }
  else {
    digitalWrite(PUMP, LOW);
  }
}

void Servo() {

  Time nowTime = pcf.getTime();//get current time

  if (nowTime.minute == 0) {
    if (nowTime.second == 0) {
      if (nowTime.hour == 6) {
        servo.write(0);
        delay(sTime);
        servo.write(40);
      }
      else if (nowTime.hour == 16) {
        servo.write(0);
        delay(sTime);
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
}
