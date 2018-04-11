#define PMS5003_SET_PIN 8 //if high -> normal mode if low  -> sleeping mode

void setup() {
  Serial.begin(9600);

  pinMode(PMS5003_SET_PIN, OUTPUT);

  digitalWrite(PMS5003_SET_PIN,LOW);
  delay(1000);
  digitalWrite(PMS5003_SET_PIN,HIGH);
}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
 
struct pms5003data data;

void loop() {

    if (! Serial.available()) {
    if (Serial.peek() != 0x42) {
      Serial.read();
      if (Serial.available() < 32) {
        if (readPMSdata()) {
          // reading data was successful!
          Serial.println();
          Serial.println("---------------------------------------");
          Serial.println("Concentration Units (standard)");
          Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
          Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_standard);
          Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
          Serial.println("---------------------------------------");
          Serial.println("Concentration Units (environmental)");
          Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
          Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
          Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
          Serial.println("---------------------------------------");
          Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
          Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
          Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
          Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
          Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
          Serial.print("Particles > 50 um / 0.1L air:"); Serial.println(data.particles_100um);
          Serial.println("---------------------------------------");
          delay(5000);
        }
      }
    }

    
  }
}

boolean readPMSdata() {
 
  uint8_t buffer[32];    
  uint16_t sum = 0;
  Serial.readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }

 /*
   //debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
  */
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
