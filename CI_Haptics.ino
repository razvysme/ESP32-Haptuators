
// =============================================================================
// Written by Razvan Paisa - 2020
// email: razvan@paisa.dk
// provided as is
// =============================================================================

/* TO DO
* change delay to millis();
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "WM8978.h"
#include "percussion.h"
#include "WiFi.h"
#include <ArduinoOSC.h>

// -----------------------------------------------------------------------------
// WiFi settings
// -----------------------------------------------------------------------------
const char* ssid     = "AirLink2b1660";
const char* password = "gQeYn7WW";
const int recv_port = 10000;
//const int send_port = 12000;
// -----------------------------------------------------------------------------
// Gloabl Variables
// -----------------------------------------------------------------------------
unsigned int delayTime = 25;
unsigned long timeNow = 0;
const float sampleRate = 48000;
String serialInput = "";
WM8978 wm8978; // the DAC
OscWiFi osc; // the OSC client
percussion faustDSP(sampleRate, 8); //the Faust synth

//temp variables
float volume = 63.0f;
int trigger = 0;
float pitch = 40;
// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  Serial.flush();

  configureDAC();
  faustDSP.start();

  scanNetworks();
  connectToNetwork();

  configureOSC(recv_port);

}
// -----------------------------------------------------------------------------
// Main Loop
// -----------------------------------------------------------------------------
void loop() 
{
  if (Serial.available()) 
  {
    serialInput = Serial.readStringUntil('\n');
    configureOverSerial();
  }
  osc.parse(); 
  //resetTrigger(delayTime);
}

String translateEncryptionType(wifi_auth_mode_t encryptionType) 
{
  switch (encryptionType) 
  {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}

void scanNetworks() 
{
  int numberOfNetworks = WiFi.scanNetworks();
  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);

  for (int i = 0; i < numberOfNetworks; i++) 
  {
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
 
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
 
    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));
 
    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
 
  }
}

void connectToNetwork() 
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }
  Serial.print("Connected to network with address: ");
  Serial.println(WiFi.localIP());
}

void configureDAC()
{
  wm8978.init();
  wm8978.addaCfg(1,1); 
  wm8978.inputCfg(1,0,0);     
  wm8978.outputCfg(1,0); 
  wm8978.micGain(30);
  wm8978.auxGain(0);
  wm8978.lineinGain(0);
  wm8978.spkVolSet(volume);
  wm8978.hpVolSet(volume,volume);
  wm8978.i2sCfg(2,0); 
}

void configureOverSerial()
{
    Serial.println(serialInput);

    if ( serialInput.startsWith( "delay " ) )
    {
      serialInput.remove( 0, 5 );
      delayTime = serialInput.substring( 0, 5 ).toInt();
      Serial.println(delayTime);
    }

    
    if ( serialInput.startsWith( "volume " ) )
    {
      serialInput.remove( 0, 6 );
      volume = serialInput.substring( 0, 6 ).toFloat();
      Serial.println(volume);
      //wm8978.spkVolSet(volume);
      wm8978.hpVolSet(volume,volume);
    }
    
    if ( serialInput.startsWith( "pitch " ) )
    {
      serialInput.remove( 0, 5 );
      pitch = serialInput.substring( 0, 5 ).toInt();
      faustDSP.setParamValue("pitch", pitch);
      Serial.println(pitch);
    }

    if ( serialInput.startsWith( "attack " ) )
    {
      serialInput.remove( 0, 6 );
      faustDSP.setParamValue("attack", serialInput.substring( 0, 5 ).toFloat());
      Serial.print("Attack changed to ");
      Serial.println(serialInput);
    }

    if ( serialInput.startsWith( "decay " ) )
    {
      serialInput.remove( 0, 5 );
      faustDSP.setParamValue("decay", serialInput.substring( 0, 5 ).toFloat());
      Serial.print("Decay changed to ");
      Serial.println(serialInput);
    }

    if ( serialInput.startsWith( "sustain " ) )
    {
      serialInput.remove( 0, 7 );
      faustDSP.setParamValue("sustain", serialInput.substring( 0, 5 ).toFloat());
      Serial.print("Sustain changed to ");
      Serial.println(serialInput);
    }

    if ( serialInput.startsWith( "release " ) )
    {
      serialInput.remove( 0, 7 );
      faustDSP.setParamValue("release", serialInput.substring( 0, 5 ).toFloat());
      Serial.print("Release changed to ");
      Serial.println(serialInput);
    }

    if ( serialInput.startsWith( "trigger" ) )
    {
      faustDSP.setParamValue("trigger", 1);
      Serial.println("triggered");
      delay(10);
      Serial.println("stopped");
      faustDSP.setParamValue("trigger", 0);
    }
}

void configureOSC(int incomingPort)
{
  osc.begin(incomingPort);
  osc.subscribe("/trigger", [](OscMessage& m)
  {
    //trigger, pitch, velocity
    faustDSP.setParamValue("pitch", m.arg<float>(0));
    faustDSP.setParamValue("velocity", m.arg<float>(1));
    //trigger = 1;
    faustDSP.setParamValue("trigger", 1);
    Serial.print("triggered with pitch: "); Serial.print(m.arg<float>(0)); Serial.print(", and velocity ");Serial.print(m.arg<float>(1)); Serial.println();
    delay(delayTime);
    faustDSP.setParamValue("trigger", 0);
  });
  

  osc.subscribe("/adsr", [](OscMessage& m)
  {
    faustDSP.setParamValue("attack", m.arg<float>(0));
    faustDSP.setParamValue("decay", m.arg<float>(1));
    faustDSP.setParamValue("sustain", m.arg<float>(2));
    faustDSP.setParamValue("release", m.arg<float>(3));

    Serial.print("ADSR changed to:  ");
    Serial.print(m.arg<float>(0)); Serial.print(" ");
    Serial.print(m.arg<float>(1)); Serial.print(" ");
    Serial.print(m.arg<float>(2)); Serial.print(" ");
    Serial.print(m.arg<float>(3)); Serial.println();
  });
}
/*
void resetTrigger(int delayTime)
{ 
  if(trigger == 1){
    if(millis() >= timeNow + delayTime)
    {
        timeNow += delayTime;
        trigger = 0;
        faustDSP.setParamValue("trigger", trigger);
        Serial.println("i waited like a good boy");
    }
  }
}
*/
// -----------------------------------------------------------------------------
// Obsolete methods
// -----------------------------------------------------------------------------
/*

void onOscReceived(OscMessage& m)
{
    //Serial.print("trigger with values: ");
    //Serial.print(m.ip()); Serial.print(" ");
    //Serial.print(m.port()); Serial.print(" ");
    //Serial.print(m.size()); Serial.print(" ");
    Serial.print(m.address()); Serial.print(" ");
    Serial.print(m.arg<int>(0)); Serial.print(" ");
    Serial.print(m.arg<float>(1)); Serial.print(" ");
}

void midiTest()
{
  MIDI.read(MidiChannel);
  //MIDI.turnThruOn();

  if (isConnected && (millis() - t1) > 1000)
  {
    t1 = millis();
    //   Serial.print(F(".");

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    MIDI.sendNoteOn(note, velocity, channel);
    MIDI.sendNoteOff(note, velocity, channel);
  }
}

void envelopeInit(float levelBegin, float levelEnd, float releaseTime) 
{
    currentLevel = levelBegin;
    coeff = ( log ( levelEnd ) - log ( levelBegin ) ) / ( releaseTime * sampleRate );
}

inline void calculateEnvelope(int samplePoints) 
{
    for (int i = 0; i < samplePoints; i++) 
    {
        currentLevel += coeff * currentLevel;
        faustSawtooth.setParamValue("gain",currentLevel);
    }
}
*/

