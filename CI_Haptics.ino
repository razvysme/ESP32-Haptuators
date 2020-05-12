
// =============================================================================
// Written by Razvan Paisa - 2020
// email: razvan@paisa.dk
// provided as is
// =============================================================================

/* TO DO
* change delay to millis();
* make wifi work
* make RPT MIDI work
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "WM8978.h"
#include "percussion.h"
#include "WiFi.h"

#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
unsigned long t1 = millis();
bool isConnected = false;
APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();


// -----------------------------------------------------------------------------
// WiFi settings
// -----------------------------------------------------------------------------
const char* ssid     = "AirLink2b1660";
const char* password = "gQeYn7WW";
//WiFiUDP Udp;
// -----------------------------------------------------------------------------
// Gloabl Variables
// -----------------------------------------------------------------------------
const float sampleRate = 48000;
float pitch = 40;
const int MidiChannel = 1;
String serialInput = "";
WM8978 wm8978; // the DAC
percussion faustDSP(sampleRate, 8);
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

  MIDI.begin(MidiChannel);
  // Stay informed on connection status
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI.setHandleReceivedMidi(OnAppleMIDIReceivedMidi);
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
  midiTest();
}

void configureOverSerial()
{
    Serial.println(serialInput);

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

void configureDAC()
{
  wm8978.init();
  wm8978.addaCfg(1,1); 
  wm8978.inputCfg(1,0,0);     
  wm8978.outputCfg(1,0); 
  wm8978.micGain(30);
  wm8978.auxGain(0);
  wm8978.lineinGain(0);
  wm8978.spkVolSet(0);
  wm8978.hpVolSet(40,40);
  wm8978.i2sCfg(2,0); 
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
  Serial.print(WiFi.localIP());
}

// =============================================================================
// Midi stuff
// =============================================================================

void midiTest()
{
  MIDI.read(MidiChannel);
  MIDI.turnThruOn();

  if (isConnected && (millis() - t1) > 1000)
  {
    t1 = millis();
    //   Serial.print(F(".");

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    //MIDI.sendNoteOn(note, velocity, channel);
    //MIDI.sendNoteOff(note, velocity, channel);
  }
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
  isConnected = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
  isConnected = false;
  Serial.println(F("Disconnected"));
}

void OnAppleMIDIReceivedMidi(const ssrc_t&, byte)
{
  Serial.println("received message ");
  //Serial.println(MIDI.read(MidiChannel));
}


// -----------------------------------------------------------------------------
// Obsolete methods
// -----------------------------------------------------------------------------
/*void envelopeInit(float levelBegin, float levelEnd, float releaseTime) 
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

