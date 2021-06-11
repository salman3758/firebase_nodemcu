#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
                          
#define FIREBASE_HOST "meter-gsm-default-rtdb.firebaseio.com"                     //Your Firebase Project URL goes here without "http:" , "\" and "/"
#define FIREBASE_AUTH "mFye9VZL4fQzootlQg1ySxwRLQSqQ3ddQsPcqoNc" //Your Firebase Database Secret goes here

#define WIFI_SSID "PTCL-BB"                                   //WiFi SSID to which you want NodeMCU to connect
#define WIFI_PASSWORD "pagal1234"                             //Password of your wifi network 
#define PIN A0
float resolution  = 3.3 / 1024;                   // Input Voltage Range is 1V to 3.3V
                                                  // ESP8266 ADC resolution is 10-bit. 2^10 = 1024

uint32_t period = 1000000 / 60;                   // One period of a periodic waveform
uint32_t t_start = 0;                                 

// setup
float zero_ADC_Value = 0, zero_voltageValue = 0;   

// loop
float ADC = 0, Vrms = 0, Current = 0, Q = 0.0147;
float sensitivity = 0.100;                        // 185 mV/A, 100 mV/A and 0.66 mV/A for ±5A, ±20A and ±30A current range respectively. 


int val=0, val3=1000;

void setup() {

  Serial.begin(115200);

  Serial.println("Serial communication started\n\n");  
           
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);


  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                            //print local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // connect to firebase
delay(1000);


pinMode(PIN, INPUT);                            // Set pin A0 as read.  
  /*--------NodeMCU--------*/
  t_start = micros();                             
  uint32_t ADC_SUM = 0, n = 0;
  while(micros() - t_start < period) {            
    ADC_SUM += analogRead(PIN);                   // reading the analog value from pin A0. 
    n++;                                          // counter to be used for avg. 
  }
  zero_ADC_Value = (ADC_SUM / n);                 // The avg analog value when no current pass throught the ACS712 sensor.
  zero_voltageValue = zero_ADC_Value * resolution;    // The ACS712 output voltage value when no current pass trough the sensor (i = 0)

}

void loop() { 

// Firebase Error Handling ************************************************
  if (Firebase.failed())
  { delay(500);
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Serial.println(Firebase.error());
  Serial.println("Connection to firebase failed. Reconnecting...");
  delay(500);
  }
  
 else { 
    Serial.println("Connected to Wifi");

   t_start = micros();                             
  uint32_t ADC_Dif = 0, ADC_SUM = 0, m = 0;        
  while(micros() - t_start < period) {            // Defining one period of the waveform. US frequency(f) is 60Hz. Period = 1/f = 0.016 seg = 16,666 microsec
    ADC_Dif = zero_ADC_Value - analogRead(PIN);   // To start from 0V we need to subtracting our initial value when no current passes through the current sensor, (i.e. 750 or 2.5V).
    ADC_SUM += ADC_Dif * ADC_Dif;                 // SUM of the square
    m++;                                          // counter to be used for avg.
  }
  ADC = sqrt(ADC_SUM / m);                        // The root-mean-square ADC value. 
  Vrms = ADC*resolution ;                       // The root-mean-square analog voltage value.   
  Current = (Vrms  / sensitivity) - Q;            // The root-mean-square analog current value. Note: Q
  
  //------------------------------//
  
  Serial.print("analogRead = ");
  Serial.println(analogRead(PIN));
  
  Serial.print("Vrms = ");                        
  Serial.print(Vrms, 6);
  Serial.println(" V");        

  Serial.print("Irms = ");                       
  Serial.print(Current, 6);
  Serial.println(" A");
  Serial.print("\n");

  Firebase.setFloat("/data",Vrms);
  Serial.println(val);
  Serial.println("uploaded Voltage to firebase\n");

  Firebase.setFloat("/data2",Current);
  Serial.println(val);
  delay(1000); Serial.println("uploaded Current to firebase \n\n");
  
  //delay(1000); 
 }

  
}
