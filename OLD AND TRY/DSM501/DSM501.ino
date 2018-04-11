#include <SPI.h>

#define DUST_SENSOR_DIGITAL_PIN_PM10  3        // DSM501 Pin 2 of DSM501 Over 1µm -> PM25
#define DUST_SENSOR_DIGITAL_PIN_PM25  5        // DSM501 Pin 4 Over 2,5µm -> PM10
#define DUSR_SENSOR_CONTROL_PIN_PM10  8        // DSM501 Pin 1


//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUSTPM25 = 0.0;
float lastDUSTPM25 = 0.0;
float valDUSTPM10 = 0.0;
float lastDUSTPM10 = 0.0;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
long concentrationPM25 = 0;
long concentrationPM10 = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM10, INPUT);
  pinMode(DUST_SENSOR_DIGITAL_PIN_PM25, INPUT);

  // wait 60s for DSM501 to warm up
//  for (int i = 1; i <= 30; i++)
//  {
//    delay(1000); // 1s
//    Serial.print(i);
//    Serial.println(" s (wait 60s for DSM501 to warm up)");
//  }

  Serial.println("Ready!");
}

void loop() {
  // put your main code here, to run repeatedly:
  //get PM 2.5 density of particles over 2.5 μm.
  concentrationPM25 = (long)getPM(DUST_SENSOR_DIGITAL_PIN_PM25);
  Serial.print("PM25: ");
  Serial.println(concentrationPM25);
  Serial.print("\n");

  //get PM 1.0 - density of particles over 1 μm.
  concentrationPM10 = getPM(DUST_SENSOR_DIGITAL_PIN_PM10);
  Serial.print("PM10: ");
  Serial.println(concentrationPM10);
  Serial.print("\n");

}

long getPM(int DUST_SENSOR_DIGITAL_PIN) {

  starttime = millis();

  while (1) {

    duration = pulseIn(DUST_SENSOR_DIGITAL_PIN, LOW);
    lowpulseoccupancy += duration;
    endtime = millis();

    if ((endtime - starttime) > sampletime_ms)
    {
      ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=>100
      //long concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
      long concentration = (2.5383 * pow(ratio, 2)) + (85.392 * ratio) - 5.8319;
      Serial.print("lowpulseoccupancy:");
      Serial.print(lowpulseoccupancy);
      Serial.print("\n");
      Serial.print("ratio:");
      Serial.print(ratio);
      Serial.print("\n");
      Serial.print("DSM501A:");
      Serial.println(concentration);
      Serial.print("\n");

      lowpulseoccupancy = 0;
      return (concentration);
    }
  }
}
