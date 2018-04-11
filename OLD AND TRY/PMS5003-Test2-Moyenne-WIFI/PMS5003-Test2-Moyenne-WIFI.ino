#define PMS5003_SET_PIN 8 //if high -> normal mode if low  -> sleeping mode

#include <SPI.h>
#include <WiFi.h>

char ssid[] = "RpiWifi"; //  your network SSID (name)
char pass[] = "pollution";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
IPAddress server(192,168,4,1);
//char server[] = "192.168.4.1";    // name address for Google (using DNS)

int comp = 0;
uint16_t pm10_sum = 0;
uint16_t pm25_sum = 0; 
uint16_t pm100_sum = 0; 
float pm10_moy = 12.5;
float pm25_moy = 4.34;
float pm100_moy = 2.49;  

String info2;

int status = WL_IDLE_STATUS;

WiFiClient client;

String sendinfo(){
  String info = "Concentration Units : PM1.0 : " + String(pm10_moy) + " PM2.5 : " + String(pm25_moy) + " PM10 : " + String(pm100_moy);
  return info;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(9600);

  pinMode(PMS5003_SET_PIN, OUTPUT);
  digitalWrite(PMS5003_SET_PIN,LOW);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 2 seconds for connection:
    delay(5000);
  }
  
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    info2 = sendinfo();
    client.print(info2);
  }
  
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
  
  if (readPMSdata()) {
    /*
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
    */
    pm10_sum += data.pm10_env;
    pm25_sum += data.pm25_env;
    pm100_sum += data.pm100_env;
    comp ++;
    if (comp >=20)
    {
      pm10_moy = (float) pm10_sum/comp;
      pm25_moy = (float) pm25_sum/comp;
      pm100_moy = (float) pm100_sum/comp;

      Serial.println("Concentration Units Mean (environmental)");
      Serial.print("PM 1.0: "); Serial.print(pm10_moy);
      Serial.print("\t\tPM 2.5: "); Serial.print(pm25_moy);
      Serial.print("\t\tPM 10: "); Serial.println(pm100_moy);
      Serial.println("---------------------------------------");

      pm10_sum = 0;
      pm25_sum = 0;
      pm100_sum = 0;
      comp = 0;
    }
    
  }
}

boolean readPMSdata() {
  if (! Serial.available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (Serial.peek() != 0x42) {
    Serial.read();
    return false;
  }
 
  // Now read all 32 bytes
  if (Serial.available() < 32) {
    return false;
  }
    
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
