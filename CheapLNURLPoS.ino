/**
 *  PIN MAP for ESP32 NODEMCU-32S, other ESP32 dev boards will vary
 *  Keypad (12-32
 *  1.8 128/160 TFT PIN MAP: [VCC - 5V, GND - GND, CS - GPIO5, Reset - GPIO16, AO (DC) - GPI17, SDA (MOSI) - GPIO23, SCK - GPIO18, LED - 3.3V]
 */
 

#include "SPI.h"
//#include "TFT_eSPI.h"
#include <Keypad.h>
#include <string.h>
#include "qrcoded.h"
#include "Bitcoin.h"
#include <Base64.h>
#include <Hash.h>
#include <Conversion.h>
#include <math.h>
#include <TFT_eSPI.h>



////////////////////////////////////////////////////////
////////CHANGE! USE LNURLPoS EXTENSION IN LNBITS////////
////////////////////////////////////////////////////////

String server = "https://legend.lnbits.com";
String posId = "TCGQL67qZbpT2ZkYgfub7p";
String key = "6CinT8TwQjkiGiTo4r8C5o";
String currency = "EUR";

////////////////////////////////////////////////////////
////Note: See lines 75, 97, to adjust to keypad size////
////////////////////////////////////////////////////////

//////////////VARIABLES///////////////////

String dataId = "";
bool paid = false;
bool shouldSaveConfig = false;
bool down = false;
const char* spiffcontent = "";
String spiffing; 
String lnurl;
String choice;
String payhash;
String key_val;
String cntr = "0";
String inputs;
int keysdec;
int keyssdec;
float temp;  
String fiat;
float satoshis;
String nosats;
float conversion;
String virtkey;
String payreq;
int randomPin;
bool settle = false;
String preparedURL;

#include "MyFont.h"

#define BIGFONT &FreeMonoBold24pt7b
#define MIDBIGFONT &FreeMonoBold18pt7b
#define MIDFONT &FreeMonoBold12pt7b
#define SMALLFONT &FreeMonoBold9pt7b
#define TINYFONT &TomThumb

TFT_eSPI tft = TFT_eSPI();
SHA256 h;

//////////////KEYPAD///////////////////

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

//Big keypad setup
//byte rowPins[rows] = {12, 13, 15, 2}; //connect to the row pinouts of the keypad
//byte colPins[cols] = {17, 22, 21}; //connect to the column pinouts of the keypad

//Small keypad setup
//byte rowPins[rows] = {21, 22, 17, 2}; //connect to the row pinouts of the keypad
//byte colPins[cols] = {15, 13, 12}; //connect to the column pinouts of the keypad

byte rowPins[rows] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[cols] = {25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );
int checker = 0;
char maxdig[20];


//////////////MAIN///////////////////

void setup(void) {
  Serial.begin(115200);
  pinMode (2, OUTPUT);
  digitalWrite(2, HIGH);
  h.begin();
  tft.begin();
  
  //Set to 3 for bigger keypad
  tft.setRotation(1);
  logo();
  delay(3000);
}

void loop() {
  inputs = "";
  settle = false;
  displaySats(); 
  bool cntr = false;
  while (cntr != true){
   char key = keypad.getKey();
   if (key != NO_KEY){
     virtkey = String(key);
       if (virtkey == "#"){
        makeLNURL();
        qrShowCode();
        int counta = 0;
         while (settle != true){
           virtkey = String(keypad.getKey());
           if (virtkey == "*"){
            tft.fillScreen(TFT_BLACK);
            settle = true;
            cntr = true;
           }
           else if (virtkey == "#"){
            showPin();
           }
         }
       }
      
      else if (virtkey == "*"){
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_WHITE);
        key_val = "";
        inputs = "";  
        nosats = "";
        virtkey = "";
        cntr = "2";
      }
      displaySats();    
    }
  }
}


///////////DISPLAY///////////////

void qrShowCode(){
  tft.fillScreen(TFT_WHITE);
  lnurl.toUpperCase();
  const char* lnurlChar = lnurl.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcoded, qrcodeData, 6, 0, lnurlChar);
    for (uint8_t y = 0; y < qrcoded.size; y++) {

        // Each horizontal module
        for (uint8_t x = 0; x < qrcoded.size; x++) {
            if(qrcode_getModule(&qrcoded, x, y)){       
                tft.fillRect(18+3*x, 1+3*y, 3, 3, TFT_BLACK);
            }
            else{
                tft.fillRect(18+3*x, 1+3*y, 3, 3, TFT_WHITE);
            }
        }
    }
}

void showPin()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(SMALLFONT);
  tft.setCursor(0, 40);
  tft.println("PROOF PIN");
  tft.setCursor(22, 80);
  tft.setTextColor(TFT_RED, TFT_BLACK); 
  tft.setFreeFont(BIGFONT);
  tft.println(randomPin);
}

void displaySats(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);      // White characters on black background
  tft.setFreeFont(SMALLFONT);
  tft.setCursor(0, 40);
  tft.println("AMOUNT THEN #");
  tft.setCursor(45, 120);
  tft.setFreeFont(TINYFONT);
  tft.println("TO RESET PRESS *");
  
  inputs += virtkey;
  float amount = float(inputs.toInt()) / 100;
  tft.setFreeFont(SMALLFONT);
  tft.setCursor(0, 70);
  tft.print(String(currency) + ":");
  tft.setFreeFont(MIDFONT);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println(amount);
  delay(100);
  virtkey = "";
}

void logo(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);      // White characters on black background
  tft.setFreeFont(MIDFONT);
  tft.setCursor(20,60);       // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  tft.print("LNURLPOS");         // Using tft.print means text background is NEVER rendered

  tft.setTextColor(TFT_PURPLE, TFT_BLACK);      // White characters on black background
  tft.setFreeFont(TINYFONT);
  tft.setCursor(45,70);       // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  tft.print("Powered by LNbits");         // Using tft.print means text background is NEVER rendered
}

void to_upper(char * arr){
  for (size_t i = 0; i < strlen(arr); i++)
  {
    if(arr[i] >= 'a' && arr[i] <= 'z'){
      arr[i] = arr[i] - 'a' + 'A';
    }
  }
}


//////////LNURL AND CRYPTO///////////////
////VERY KINDLY DONATED BY SNIGIREV!/////

void makeLNURL(){
  randomPin = random(1000,9999);
  byte nonce[8];
  for(int i = 0; i < 8;i++){
    nonce[i] = random(9);
  }
  byte payload[8];
  encode_data(payload, nonce, randomPin, inputs.toInt());
  preparedURL = server + "/lnurlpos/api/v1/lnurl/";
  preparedURL += toHex(nonce,8);
  preparedURL += "/";
  preparedURL += toHex(payload, 8);
  preparedURL += "/";
  preparedURL += posId;
  Serial.println(preparedURL);
  char Buf[200];
  preparedURL.toCharArray(Buf, 200);
  char *url = Buf;
  byte * data = (byte *)calloc(strlen(url)*2, sizeof(byte));
  size_t len = 0;
  int res = convert_bits(data, &len, 5, (byte *)url, strlen(url), 8, 1);
  char * charLnurl = (char *)calloc(strlen(url)*2, sizeof(byte));
  bech32_encode(charLnurl, "lnurl", data, len);
  to_upper(charLnurl);
  lnurl = charLnurl;
  Serial.println(lnurl);
}

void encode_data(byte output[8], byte nonce[8], int pin, int amount_in_cents){
  SHA256 h;
  h.write(nonce, 8);
  h.write((byte *)key.c_str(), key.length());
  h.end(output);
  output[0] = output[0] ^ ((byte)(pin & 0xFF));
  output[1] = output[1] ^ ((byte)(pin >> 8));
  for(int i=0; i<4; i++){
    output[2+i] = output[2+i] ^ ((byte)(amount_in_cents & 0xFF));
    amount_in_cents = amount_in_cents >> 8;
  }
}
