// LCD displeja pieslēgšana shēma:
// LCD:     DB7       -> Arduino Digital 7
//          DB6       -> Arduino Digital 6 
//          DB5       -> Arduino Digital 5
//          DB4       -> Arduino Digital 4
//          E         -> Arduino Digital 9
//          RS        -> Arduino Digital 8

// DS3131 moduļa pieslēgšana SDA - Arduino Analog 4; SLC - Arduino Analog 5

// Bibliotēku pievienošana:
#include <Wire.h>                            // bibliotēka darbam ar kopni I2C
#include <LiquidCrystal.h>                   // bibliotēka darbam ar LCD displeju
#include <iarduino_RTC.h>                    // bibliotēka darbam ar RTC moduli
LiquidCrystal lcd(8,   9,  4,  5,  6,  7);   // objekta izveidošana darbam ar LCD displeju (pieslēgšanas pini)
iarduino_RTC time(RTC_DS3231);               // objekta izveidošana darbam ar RTC moduli (RTC_moduļa nosaukums)

// Mainīgo deklarēšana
uint8_t VAR_mode_SHOW   = 1;                 // izvades režīms: 1-laiks 2-datums 3-gaismaON laiks 4--gaismaOFF laiks
uint8_t VAR_mode_SET    = 0;                 // laika iestātījuma režīms: 0-nav 1-sek 2-min 3-stunda 4-datums 5-mēn 6-gads 7-ned_diena 8-min_mod 9-stunda_mod
uint8_t VAR_alarm_MIN   = 40;                // ALARM minutes
uint8_t VAR_alarm_HOUR  = 11;                // ALARM stundas
uint8_t VAR_off_MIN     = 50;                // OFF minutes
uint8_t VAR_off_HOUR    = 11;                // OFF stundas
uint8_t VAR_night_ON_MIN  = 30;
uint8_t VAR_night_ON_HOUR = 20;              // nakts režīma sākuma minutes
uint8_t VAR_night_OFF_MIN = 59;
uint8_t VAR_night_OFF_HOUR= 23;              // nakts režīma sākuma stundas
bool    VAR_alarm_FLAG1 = true;              // modinatajs - darbības atļaušana
bool    VAR_alarm_FLAG2 = false;             // modinatajs - darbības atļaušana (ieslēgšana)
bool    VAR_alarm_FLAG3 = false;             // modinatajs - darbības atļaušana (izslēgšana)
bool    VAR_night_FLAG1 = true;              // nakts režīma atļaušana
bool    VAR_night_FLAG2 = false;             // nakts režīma atļaušana
#define PIR_PIN    2                         // direktīva kura nosaka identifikatora vērtību - PIR sensora pina numuru
#define R_LED_PIN  3                         // direktīva kura nosaka identifikatora vērtību - R_LED pina numuru
#define LCD_BR_PIN 10                        // direktīva kura nosaka identifikatora vērtību - LCD displeja apgaismes pina numuru
#define B_LED_PIN  11                        // direktīva kura nosaka identifikatora vērtību - B_LED pina numuru  
byte    MAS_alarm_SYM[8]={B00000,B01110,B10101,B10111,B10001,B01110,B00000,B00000};  // modinataja simbols
int     brightness       = 0;                // mainīgais apgaismojuma spilgtuma glabāšanai
int     LCD_br           = 255;              // mainīgais displeja apgaismojuma spilgtuma glabāšanai
int     val              = 0;                // mainīgais nakts režīma darbībai
int     night_brightness = 50;               // mainīgais nakts apgaismojuma spilgtuma glabāšanai
int     i;                                   // mainīgais nakts režīma darbībai
unsigned long previousMillis = 0;            // mainīgais taimera laika saglabāšanai
unsigned long interval       = 200;          // laika intervāls pēc kura pieaug nakts režīma apgaismojums

// LCD paneļa pogu definēšana
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0                          // direktīva kura nosaka identifikatora vērtību
#define btnUP     1                          // direktīva kura nosaka identifikatora vērtību
#define btnDOWN   2                          // direktīva kura nosaka identifikatora vērtību
#define btnLEFT   3                          // direktīva kura nosaka identifikatora vērtību
#define btnSELECT 4                          // direktīva kura nosaka identifikatora vērtību
#define btnNONE   5                          // direktīva kura nosaka identifikatora vērtību

int read_LCD_buttons(){                      // funkcija spiežampogu nolasīšanai
  adc_key_in = analogRead(0);                // nolasīt vērtību no analoga ieejas 0 
  if (adc_key_in > 1000) return btnNONE;     // ja vērtība > 1000, tad atgriezt - btnNONE
  if (adc_key_in < 50)   return btnRIGHT;    // ja vērtība < 50, tad atgriezt   - btnRIGHT
  if (adc_key_in < 250)  return btnUP;       // ja vērtība < 250, tad atgriezt  - btnUP
  if (adc_key_in < 450)  return btnDOWN;     // ja vērtība < 450, tad atgriezt  - btnDOWN
  if (adc_key_in < 650)  return btnLEFT;     // ja vērtība < 650, tad atgriezt  - btnLEFT
  if (adc_key_in < 850)  return btnSELECT;   // ja vērtība < 850, tad atgriezt  - btnSELECT
  return btnNONE;                            // ja neizpildas neviens nosacījums, tad atgriezt - btnNONE
}

// Aparatūras iestatīšana
void setup() {

  Serial.begin(19200);                       // virknes pieslēguma inicializācija (ātrums bit/s)
  pinMode(R_LED_PIN,  OUTPUT);               // pina režīma iestatīšana - izeja
  pinMode(LCD_BR_PIN, OUTPUT);               // pina režīma iestatīšana - izeja  // displeja apgaismojums
  pinMode(B_LED_PIN,  OUTPUT);               // pina režīma iestatīšana - izeja
  pinMode(PIR_PIN,    INPUT);                // pina režīma iestatīšana - ieeja
  
  delay(300);                                // pārtraukums
  time.begin();                              // RTC moduļa inicializācija
  //time.settime(0,51,21,27,10,15,2);        // laika iestatīšana (sek, min, st, dat, men, gads, ned_diena)
  lcd.begin(16, 2);                          // LCD displeja inicializācija ( 16 ailes, 2 rindas)
  lcd.createChar(1, MAS_alarm_SYM);          // ielādēt modinātāja simbolu ar numuru 1
  analogWrite(LCD_BR_PIN, LCD_br);
  previousMillis = millis();
}

//Galvenā programma
void loop(){

  if(millis()%1000==0){                      // ja pagāja 1 sekunde
    if(VAR_mode_SET==0){                     // ja laiks/datums/modinātājs tiek izvādīts uz displeja, nevis tiek iestātīts
      lcd.setCursor(0, 0);                   // uzstādīt kursoru pozicijā 0. aile, 0. rinda
      if(VAR_mode_SHOW==1){lcd.print("Laiks"     );} // ja izvades režīms = 1, izvadīt tekstu "Laiks"
      if(VAR_mode_SHOW==2){lcd.print("Datums"    );} // ja izvades režīms = 2, izvadīt tekstu "Datums"
      if(VAR_mode_SHOW==3){lcd.print("Gaisma ON" );} // ja izvades režīms = 3, izvadīt tekstu "Gaisma ON"
      if(VAR_mode_SHOW==4){lcd.print("Gaisma OFF");} // ja izvades režīms = 4, izvadīt tekstu "Gaisma OFF"
      lcd.setCursor(15, 0);                  // uzstādīt kursoru pozicijā 15. aile, 0. rinda
      lcd.print(VAR_alarm_FLAG1?"\1":" ");   // izvadīt modinātāja simbolu
    }else{                                   // ja laiks/datums/modinātājs tiek iestātīts, nevis tiek izvādīts uz displeja
      lcd.setCursor(0, 0);                   // uzstādīt kursoru pozicijā 0. aile, 0. rinda
      if(VAR_mode_SHOW==1){lcd.print("Uzst. laiku:"    );}
      if(VAR_mode_SHOW==2){lcd.print("Uzst. datumu:"   );}
      if(VAR_mode_SHOW==3){lcd.print("Uzst. GaismaON:" );}
      if(VAR_mode_SHOW==4){lcd.print("Uzst. GaismaOFF:");}
    }
    if(VAR_mode_SHOW==1){                    // ja ir laika izvades režīms
      lcd.setCursor(0, 1);                   // uzstādīt kursoru pozicijā 0. aile, 1. rinda
      time.gettime();                        // atjaunot mainīga time.seconds vērtību - priekš regulējamā parametra mirgošanai
      if(VAR_mode_SET==3&&time.seconds%2){lcd.print("  ");}else{lcd.print(time.gettime("H"));}
      lcd.print(":");
      if(VAR_mode_SET==2&&time.seconds%2){lcd.print("  ");}else{lcd.print(time.gettime("i"));}
      lcd.print(":");
      if(VAR_mode_SET==1&&time.seconds%2){lcd.print("  ");}else{lcd.print(time.gettime("s"));}        
    }
    if(VAR_mode_SHOW==2){                    // ja ir datuma izvades režīms
      lcd.setCursor(0, 1);                   // uzstādīt kursoru pozicijā 0. aile, 1. rinda
      time.gettime();                        // atjaunot mainīga time.seconds vērtību - priekš regulējamā parametra mirgošanai
      if(VAR_mode_SET==4&&time.seconds%2){lcd.print("  ") ;}else{if(time.day  <10){lcd.print("0");} lcd.print(time.day  );}
      lcd.print("-");
      if(VAR_mode_SET==5&&time.seconds%2){lcd.print("  ") ;}else{if(time.month<10){lcd.print("0");} lcd.print(time.month);}
      lcd.print("-20");
      if(VAR_mode_SET==6&&time.seconds%2){lcd.print("  ") ;}else{if(time.year <10){lcd.print("0");} lcd.print(time.year );}
      lcd.print("   ");
      if(VAR_mode_SET==7&&time.seconds%2){lcd.print("   ");}else{lcd.print(time.gettime("D"));}
    }
    if(VAR_mode_SHOW==3){                    // ja ir modinātāja(ON) izvades režīms
      lcd.setCursor(0, 1);                   // uzstādīt kursoru pozicijā 0. aile, 1. rinda
      time.gettime();                        // atjaunot mainīga time.seconds vērtību - priekš regulējamā parametra mirgošanai
      if(VAR_mode_SET==9&&time.seconds%2){lcd.print("  ");}else{if(VAR_alarm_HOUR<10){lcd.print("0");} lcd.print(VAR_alarm_HOUR);}
      lcd.print(":");
      if(VAR_mode_SET==8&&time.seconds%2){lcd.print("  ");}else{if(VAR_alarm_MIN<10){ lcd.print("0");} lcd.print(VAR_alarm_MIN) ;}
    }
    if(VAR_mode_SHOW==4){                    // ja ir modinātāja(OFF) izvades režīms
      lcd.setCursor(0, 1);                   // uzstādīt kursoru pozicijā 0. aile, 1. rinda
      time.gettime();                        // atjaunot mainīga time.seconds vērtību - priekš regulējamā parametra mirgošanai
      if(VAR_mode_SET==11&&time.seconds%2){lcd.print("  ");}else{if(VAR_off_HOUR<10){lcd.print("0");} lcd.print(VAR_off_HOUR);}
      lcd.print(":");
      if(VAR_mode_SET==10&&time.seconds%2){lcd.print("  ");}else{if(VAR_off_MIN<10){ lcd.print("0");} lcd.print(VAR_off_MIN) ;}
    }
    delay(1);                                // pārtraukums 1ms

// modinātāja (Gaisma ON) pārbaude
    if(VAR_alarm_FLAG1){                     // ja modinātājs ieslēgts
      if(time.seconds==00){                  // ja tagad ir 00 sekundes
        if(time.minutes==VAR_alarm_MIN){     // ja sakrīt minutes
          if(time.Hours==VAR_alarm_HOUR){    // ja sakrīt stundas
            VAR_alarm_FLAG2=true;            // atļaut modinātāja nostrādi
          }
        }
      }
    }else{VAR_alarm_FLAG2=false;}            // ja modinātājs ir izslēgts, aizliegt modinātāja nostrādi
    if(VAR_alarm_FLAG2){Func_alarm_action();}// palaist modinātāja funkciju

// modinātāja (Gaisma OFF) pārbaude
    if(VAR_alarm_FLAG1){                     // ja modinātājs ieslēgts
      if(time.seconds==00){                  // ja tagad ir 00 sekundes
        if(time.minutes==VAR_off_MIN){       // ja sakrīt minutes
          if(time.Hours==VAR_off_HOUR){      // ja sakrīt stundas
            VAR_alarm_FLAG3=true;            // atļaut modinātāja nostrādi
          }
        }
      }
    }else{VAR_alarm_FLAG3=false;}            // ja modinātājs ir izslēgts, aizliegt modinātāja nostrādi
    if(VAR_alarm_FLAG3){Func_off_action();}  // palaist modinātāja funkciju


// nakts režīma pārbaude un ieslēgšana
    if(VAR_night_FLAG1){                     // ja nakts režīms ir ieslēgts
      if(time.seconds>=00){                  // ja tagad ir 00 sekundes
        if(time.minutes>=VAR_night_ON_MIN){  // ja sakrīt minutes
          if(time.Hours>=VAR_night_ON_HOUR){ // ja sakrīt stundas
            VAR_night_FLAG2=true;            // atļaut nakts režīma nostrādi
          }
        }
      }
    }

// nakts režīma pārbaude un izslēgšana
    if(VAR_night_FLAG1){                     // ja nakts režīms ir ieslēgts
      if(time.seconds==00){                  // ja tagad ir 00 sekundes
        if(time.minutes==VAR_night_OFF_MIN){ // ja sakrīt minutes
          if(time.Hours==VAR_night_OFF_HOUR){// ja sakrīt stundas
            VAR_night_FLAG2=false;           // aizliegt nakts režīma nostrādi
          }
        }
      }
    }
    if(VAR_night_FLAG2){Func_night_action();} // palaist nakts režīma funkciju
  }
  
  Func_buttons_control();                     // pogu vadības funkcija
}

// pogu vadības funkcija
void Func_buttons_control(){
  lcd_key = read_LCD_buttons();               // nolasīt kura poga ir nospiesta
  uint8_t i=0;
  if(VAR_mode_SET){                           // ja izvades režīms - laika/datuma iestatīšana
   switch (lcd_key){                          // atkarībā no tā, kura poga ir nospiesta
    
    case btnUP:                               // ja nospiesta poga btnUP
     { 
        if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)// ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
        {
          VAR_alarm_FLAG2=false;              // deaktivizēt funkciju Gaisma ON
          VAR_alarm_FLAG3=false;              // deaktivizēt funkciju Gaisma OFF
          delay(500);
          break;
          }else{                              
        switch (VAR_mode_SET){                // iestatāmā parametra inkrements
        /* sek */ case 1: time.settime(0,                                   -1, -1, -1, -1, -1, -1); break;
        /* min */ case 2: time.settime(-1, (time.minutes==59?0:time.minutes+1), -1, -1, -1, -1, -1); break;
        /* stu */ case 3: time.settime(-1, -1, (time.Hours==23?0:time.Hours+1),     -1, -1, -1, -1); break;
        /* die */ case 4: time.settime(-1, -1, -1, (time.day==31?1:time.day+1),         -1, -1, -1); break;
        /* men */ case 5: time.settime(-1, -1, -1, -1, (time.month==12?1:time.month+1),     -1, -1); break;
        /* gad */ case 6: time.settime(-1, -1, -1, -1, -1, (time.year==99?0:time.year+1),       -1); break;
        /* n.d.*/ case 7: time.settime(-1, -1, -1, -1, -1, -1, (time.weekday==6?0:time.weekday+1) ); break;
        /* m.m.*/ case 8:  VAR_alarm_MIN =VAR_alarm_MIN ==59?0:VAR_alarm_MIN +1;                     break;
        /* m.s.*/ case 9:  VAR_alarm_HOUR=VAR_alarm_HOUR==23?0:VAR_alarm_HOUR+1;                     break;
        /* m.m.*/ case 10: VAR_off_MIN   =VAR_off_MIN   ==59?0:VAR_off_MIN   +1;                     break;
        /* m.s.*/ case 11: VAR_off_HOUR  =VAR_off_HOUR  ==23?0:VAR_off_HOUR  +1;                     break;
        }
        delay(500);
        break;
      }
     }
   
   case btnDOWN:                              // ja nospiesta poga btnDOWN
     {
        if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)// ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
        {
          VAR_alarm_FLAG2=false;              // deaktivizēt funkciju Gaisma ON
          VAR_alarm_FLAG3=false;              // deaktivizēt funkciju Gaisma OFF
          delay(500);
          break;
        }else{                                  
        switch (VAR_mode_SET){                // iestatāmā parametra inkrements
        /* sek */ case 1: time.settime(0,                                   -1, -1, -1, -1, -1, -1); break;
        /* min */ case 2: time.settime(-1, (time.minutes==0?59:time.minutes-1), -1, -1, -1, -1, -1); break;
        /* stu */ case 3: time.settime(-1, -1, (time.Hours==0?23:time.Hours-1),     -1, -1, -1, -1); break;
        /* die */ case 4: time.settime(-1, -1, -1, (time.day==1?31:time.day-1),         -1, -1, -1); break;
        /* men */ case 5: time.settime(-1, -1, -1, -1, (time.month==1?12:time.month-1),     -1, -1); break;
        /* gad */ case 6: time.settime(-1, -1, -1, -1, -1, (time.year==0?99:time.year-1),       -1); break;
        /* n.d.*/ case 7: time.settime(-1, -1, -1, -1, -1, -1, (time.weekday==0?6:time.weekday-1) ); break;
        /* m.m.*/ case 8:  VAR_alarm_MIN =VAR_alarm_MIN ==0?59:VAR_alarm_MIN -1;                     break;
        /* m.s.*/ case 9:  VAR_alarm_HOUR=VAR_alarm_HOUR==0?23:VAR_alarm_HOUR-1;                     break;
        /* m.m.*/ case 10: VAR_off_MIN   =VAR_off_MIN   ==59?0:VAR_off_MIN   -1;                     break;
        /* m.s.*/ case 11: VAR_off_HOUR  =VAR_off_HOUR  ==23?0:VAR_off_HOUR  -1;                     break;
        }
        delay(500);
        break;
      }
     }
   
   case btnRIGHT:                             // ja nospiesta poga btnRIGHT
     {
      lcd.clear();                            // notīrīt LCD ekrānu
      if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)  // ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
      {
        VAR_alarm_FLAG2=false;                // deaktivizēt funkciju Gaisma ON
        VAR_alarm_FLAG3=false;                // deaktivizēt funkciju Gaisma OFF
        delay(500);
        break;
       }else{if (VAR_mode_SHOW==1){VAR_mode_SHOW=2; VAR_mode_SET=4; }  // pārslēgties uz nākāmo ekrānu
        else{if (VAR_mode_SHOW==2){VAR_mode_SHOW=3; VAR_mode_SET=8; }  // pārslēgties uz nākāmo ekrānu
        else{if (VAR_mode_SHOW==3){VAR_mode_SHOW=4; VAR_mode_SET=10;}  // pārslēgties uz nākāmo ekrānu
        else{if (VAR_mode_SHOW==4){VAR_mode_SHOW=1; VAR_mode_SET=0; }  // pārslēgties uz nākāmo ekrānu
       }
       }
       }
       }
       delay(500);
       break;
     }
      

    case btnSELECT:                            // ja nospiesta poga btnSELECT
     {
      if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)   // ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
        {
          VAR_alarm_FLAG2=false;               // deaktivizēt funkciju Gaisma ON
          VAR_alarm_FLAG3=false;               // deaktivizēt funkciju Gaisma OFF
          delay(500);
          break;
          }else{
            lcd.clear();                       // notīrīt LCD ekrānu
            VAR_mode_SET++;                    // pārslēgties uz nākāmo parametru
            if(VAR_mode_SET>11){VAR_mode_SET=0;   VAR_mode_SHOW=1;} // pārslēgties uz nākāmo ekrānu
            if(VAR_mode_SET>0 && VAR_mode_SET<4) {VAR_mode_SHOW=1;} // pārslēgties uz nākāmo ekrānu
            if(VAR_mode_SET>3 && VAR_mode_SET<8) {VAR_mode_SHOW=2;} // pārslēgties uz nākāmo ekrānu
            if(VAR_mode_SET>7 && VAR_mode_SET<10){VAR_mode_SHOW=3;} // pārslēgties uz nākāmo ekrānu
            if(VAR_mode_SET>9 && VAR_mode_SET<12){VAR_mode_SHOW=4;} // pārslēgties uz nākāmo ekrānu
            delay(500);                        // aizsture
            break;
          }
      }   
  }

  }else{                                       // ja izvades režīms - laika/datuma izvade

   switch (lcd_key)                            // atkarībā no tā, kura poga ir nospiesta
 {
   case btnUP:                                 // ja nospiesta poga btnUP
     {
        if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3) // ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
        {
          VAR_alarm_FLAG2=false;               // deaktivizēt funkciju Gaisma ON
          VAR_alarm_FLAG3=false;               // deaktivizēt funkciju Gaisma OFF
          delay(500);
          break;
         }else{                                
        if(VAR_mode_SHOW==1 || VAR_mode_SHOW==2)// ja izvades režīms - laika vai datuma izvade
        {
          if(brightness>239)
            {
              brightness = 10;
              analogWrite(R_LED_PIN, brightness);
              delay(500);
              brightness = 255;
              analogWrite(R_LED_PIN, brightness);
              delay(500);
              break;
            }
          brightness = (brightness + 25) % 256;  // palielināt apgaismojuma spilgtumu 
          analogWrite(R_LED_PIN, brightness);
          delay(500);
          break;
         }
        if(VAR_mode_SHOW==3 || VAR_mode_SHOW==4)// ja izvades režīms - modinātāja laika izvade
        {
          VAR_alarm_FLAG1=VAR_alarm_FLAG1?0:1;  // ieslēgt/izslēgt modinātāju
          delay(500);
          break;
         }  
      }
     }
   
   case btnDOWN:                                // ja nospiesta poga btnDOWN
     {
        if(VAR_mode_SHOW==1 || VAR_mode_SHOW==2)// ja izvades režīms - laika vai datuma izvade
        {
          if(brightness<11)
            {
              brightness = 0;
              analogWrite(R_LED_PIN, brightness);
              delay(500);
              break;
            }
          brightness = (brightness - 25);  // palielināt apgaismojuma spilgtumu 
          analogWrite(R_LED_PIN, brightness);
          delay(500);
          break;
         }
        if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)  // ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
        {
          VAR_alarm_FLAG2=false;                // deaktivizēt funkciju Gaisma ON
          VAR_alarm_FLAG3=false;                // deaktivizēt funkciju Gaisma OFF
          delay(500);
          break;
         }else{
        if(VAR_mode_SHOW==3 || VAR_mode_SHOW==4)// ja izvades režīms - modinātāja laika izvade
        {
          VAR_alarm_FLAG1=VAR_alarm_FLAG1?0:1;  // ieslēgt/izslēgt modinātāju
          delay(500);
          break;
         } 
      }
     }
   
   case btnRIGHT:                               // ja nospiesta poga btnRIGHT
      {
        if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)  // ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
        {
          VAR_alarm_FLAG2=false;                // deaktivizēt funkciju Gaisma ON
          VAR_alarm_FLAG3=false;                // deaktivizēt funkciju Gaisma OFF
          delay(500);
          break;
         }else{
          lcd.clear();                          // notīrīt LCD ekrānu
          VAR_mode_SHOW++;                      // pārslēgt uz nākāmo ekrānu
          if(VAR_mode_SHOW>4){VAR_mode_SHOW=1;} // ja ir pēdējais ekrāns, pārslēgt uz pirmo
          delay(500);
          break;
          }
       }

  case btnSELECT:                               // ja nospiesta poga btnSELECT
       {
        if(VAR_alarm_FLAG2 || VAR_alarm_FLAG3)  // ja aktivizēta funkcija Gaisma ON vai Gaisma OFF
          {
            VAR_alarm_FLAG2=false;              // deaktivizēt funkciju Gaisma ON
            VAR_alarm_FLAG3=false;              // deaktivizēt funkciju Gaisma OFF
            delay(500);
            break;
          } else{
                  lcd.clear();                  // notīrīt LCD ekrānu
                  VAR_mode_SET++;               // pārslēgt uz nākāmo parametru
                  if(VAR_mode_SET>11) {VAR_mode_SET=0; } // ja ir pēdējais parametrs, iziet no iestātīšanas režīma
                  if(VAR_mode_SET==1) {VAR_mode_SHOW=1;} // ja tiek iestātīts 1 parametrs, pārslēgt uz 1 ekrānu
                  delay(500);                            // aizsture
                  break;
                }
        }
  }
}

  switch (lcd_key)                              // neatkarīgi kurš ekrāns tiek izvādīts
  {
    case btnLEFT:                               // ja nospiesta poga btnLEFT
      {
          LCD_br = (LCD_br - 64);               // samazināt displeja apgaismojuma spilgtumu 
          analogWrite(LCD_BR_PIN, LCD_br);
          delay(500);
          break;
      }
  }

}

void Func_alarm_action()                       // funkcija Gaisma On
{
    brightness = (brightness + 10);            // palielināt apgaismojuma spilgtumu
    analogWrite(R_LED_PIN, brightness);
    delay(500);
    if(brightness>230)                         // ja spilgtums kļūst lielāks par 230
      {
        brightness=255;                        // uzstādīt maksimālo spilgtumu
        analogWrite(R_LED_PIN, brightness); 
        VAR_alarm_FLAG2=false;                 // iziet no funkcijas
      }
}

void Func_off_action()                          // funkcija Gaisma OFF
{
    brightness = (brightness - 10);             // samazināt apgaismojuma spilgtumu
    analogWrite(R_LED_PIN, brightness);
    delay(500);
    if(brightness<11)                           // ja spilgtums kļūst mazāks par 11
      {
        brightness=0;                           // izslēgt apgaismojumu
        analogWrite(R_LED_PIN, brightness);
        VAR_alarm_FLAG3=false;                  // iziet no funkcijas
       }
}

void Func_night_action()                        // funkcija nakts režīms
{
  val = digitalRead(PIR_PIN);                   // nolasīt PIR sensora stāvokli
  if(i==0 && val==HIGH)                         // ja stāvoklis ir HIGH
  {
    do
      {
        if (millis() - previousMillis > interval)// ja pagājis laiks lielāks par intervālu
        {
          i++;                                   // palielināt spilgtumu
          analogWrite(R_LED_PIN, i);             // ieslēgt apgaismojumu
          previousMillis=millis();               // ierakstīt laiku 
        }
      } while (i<night_brightness);              // atkārtot kamēr nesasniedz night_brightness
  } else
  {
    if(i!=0 && val==LOW)                         // ja apgaismojums ir ieslēgts
    {
      do
      {
        if (millis() - previousMillis > interval)// ja pagājis laiks lielāks par intervālu
        {
          i--;                                   // samazināt spilgtumu
          analogWrite(R_LED_PIN, i);
          previousMillis=millis();
        }
      } while (i!=0);                            // atkārtot kamēr nesasniedz 0
    }
  }
}
