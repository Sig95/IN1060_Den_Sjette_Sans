// Den Sjette Sans
// av Andrea Rye, Marthe Seth, Lara Silberhorn, Sigrid Strand Stiberg, Pernille Vannebo og Hanna Karsrud

// Pins til ultrasonisk sensor
const int triggerPin = 50;                          // sender ut signal
const int echoPin = 52;                             // tar imot signal
long etappeTid;                                     // tiden det tar for den ultrasoniske bølgen å gå fra trigger til echo
int avstand;                                        // avstand (i cm)som sensor måler
const int grenseAvstand = 80;                       // avstand i cm. Om ultrasonisk sensor detekterer avstand større enn dette, settes eget rgb lys til hvitt
unsigned long startTid;                             // settes til millis() i det US sensor måler lengre avstand enn grenseAvstand
const unsigned long grenseTid = 900 * 1000;         // hvis bruker ikke sitter ved pulten etter grenseTid, vil status automatisk settes til "ikke tilstede". 
                                                        // Angis i sekunder * 1000 (for å gjøre om til millisekunder)

// Pins til reed switcher
const int reed1 = 20;                               // kobles til pins med interrupt funksjon (pin 2, 3, 18, 19, 20, 21 på arduino mega)  (KILDE HER!!!!!!)    
const int reed2 = 21;       
const int reed3 = 2; 

// Pins egen RGB-led
const int r_egen = 4;
const int g_egen = 5;
const int b_egen = 6;
int statuskode;                                     // holder styr på brukers egen status,  1=opptatt   2=samarbeid   3=pause  

const unsigned long opptattPaaminnelse = 1800000;   // hvor lenge det skal gå før bruker får påminnelse om at de har statusen sin på opptatt 
unsigned long startTidOpptatt = 0;                  // tidspunkt når bruker plasserer magnet på opptatt-status

// Pins til de 3 rgb ledene som representerer team-medlemmer. Navn nord, sor og vest korresponderer med plasseringer deres på artefakten
const int r_nord = A2;
const int g_nord = A1;
const int b_nord = A0;
const int r_sor = A3;
const int g_sor = A5;
const int b_sor = A4;
const int r_vest = A8;
const int g_vest = A6;
const int b_vest = A7;

unsigned long tidSkiftetFarge = millis();         // tidspunkt når en av rgb ledene skiftet farge sist
const unsigned long rgb_grensetid = 8 * 1000;     // lengde tid før ny rgb led skal skifte farge
int teller = 0;                                   // teller for fargemonster array i skiftFarge() metode
int forrigeRGB = 1;                               // holder styr på  hvilken rgb som byttet farge siste

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(reed1, INPUT_PULLUP);
  pinMode(reed2, INPUT_PULLUP);
  pinMode(reed3, INPUT_PULLUP);
  // om en av reed switchene aktiveres, vil programmet automatisk hoppe til korresponderende
  // funksjon (settOpptatt, settSamarbeid eller settPause) og utføre dette
  attachInterrupt(digitalPinToInterrupt(reed1), settOpptatt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(reed2), settSamarbeid, CHANGE);
  attachInterrupt(digitalPinToInterrupt(reed3), settPause, CHANGE);

  pinMode(r_egen, OUTPUT);
  pinMode(g_egen, OUTPUT);
  pinMode(b_egen, OUTPUT);
  
  //Kode som skal utføres kun 1 gang (ved oppstart)
  // sjekker om magneten er plassert på en reed switch. Hvis ja, så settes statuskode til korresponderende tall 
  avgjorEgenStatus();
  //setter rgb lysene til å lyse
  skiftFargeVestSor(1, r_sor, g_sor, b_sor);
  skiftFargeVestSor(3, r_vest, g_vest, b_vest);
  skiftFargeNord(4, r_nord, g_nord, b_nord);
}

void loop() {
  // endrer statuslys basert på verdien til statuskode
  settEgenStatus();
  // ultrasonisk sensor trigges og måler avstand
  trigUltrasoniskSensor();
  // sjekker om bruker er tilstede eller ikke
  sjekkTilstedevaerelse();

  //sjekker om bruker har hatt magnet på "opptatt" over grensetiden. Hvis ja, starter pulsering.
  if ((millis() - startTidOpptatt) > opptattPaaminnelse){
    pulsering();
  }
  // sjekker om det har gått mer tid enn grensetiden, siden siste en rgb led skiftet farge. 
  //Hvis ja, skifter farge og oppdaterer tiden dette skjedde
  if((millis() - tidSkiftetFarge) >= rgb_grensetid){
    skiftFarge();
    tidSkiftetFarge = millis();
  }
}

// De tre "sett"-metodene blir aktivert ved hjelp av interrupt funksjonaliteten på arduino brettet. Når en av de trigges, 
// sjekkes reed-swtichen sin status, statuskoden oppdateres og settEgenStatus() blir kallt på for å oppdatere lyset
void settOpptatt(){
  if(digitalRead(reed1) == LOW){
    statuskode = 1;
  }
  else{
    statuskode = 0;
  }
  settEgenStatus();  
}

void settSamarbeid(){
  if(digitalRead(reed2) == LOW){
    statuskode = 2;
  }
  else{
    statuskode = 0;
  }
  settEgenStatus();  
}

void settPause(){
  if(digitalRead(reed3) == LOW){
    statuskode = 3;
  }
  else{
    statuskode = 0;
  }
  settEgenStatus();  
}

// metoden sjekker statuskoden og endrer brukerens rgb led til riktig farge
// startTidOpptatt oppdateres til nåværende tid hvis statusen endres til opptatt, mens den nullstilles
// hvis statusen endres til noe annet
void settEgenStatus(){
  // opptatt - rød
  if(statuskode == 1){      
    analogWrite(r_egen, 0);
    analogWrite(g_egen, 255);
    analogWrite(b_egen, 255);
    if(startTidOpptatt == 0){
      startTidOpptatt = millis();
    }
  }
  // samarbeid - gul
  else if(statuskode == 2){
    analogWrite(r_egen, 0);
    analogWrite(g_egen, 0);
    analogWrite(b_egen, 255);
    startTidOpptatt = 0;
  }
  // pause - blå
  else if (statuskode == 3){
    analogWrite(r_egen, 255);
    analogWrite(g_egen, 255);
    analogWrite(b_egen, 0);
    startTidOpptatt = 0;
  }
  // tilstede - grønn
  else if (statuskode == 0){
    analogWrite(r_egen, 255);
    analogWrite(g_egen, 0);
    analogWrite(b_egen, 255);
    startTidOpptatt = 0;
  }
  // ikke tilstede - hvit
  else if (statuskode == 4){
    analogWrite(r_egen, 0);
    analogWrite(g_egen, 0);
    analogWrite(b_egen, 0);
  }  
}


void trigUltrasoniskSensor(){
  // koden er basert på kode eksempelet på arduino sine nettsider
  /************************************************************************
  * Tittel: Ping Ultrasonic Range Finder
  * Dato: 16.05.2023
  * Lenke: https://docs.arduino.cc/built-in-examples/sensors/Ping 
  /************************************************************************/
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delay(10);
  digitalWrite(triggerPin, LOW);
  //leser av tiden det tar for echoPin å registrere signalet
  etappeTid = pulseIn(echoPin, HIGH);
  // regner om til avstand i cm
  avstand = etappeTid * 0.034/2;
}

void sjekkTilstedevaerelse(){
  //nullstiller timer om personen har kommet tilbake til plassen før tidsgrensen har passert
  if (startTid > 0 && avstand < grenseAvstand){
    startTid = 0;
    avgjorEgenStatus();
  }
  // hvis nåværende avstand er over grense og timer ikke har startet
  // start timer (sett start tid til nåværende tid)
  else if (avstand > grenseAvstand && startTid == 0){
    startTid = millis();
    avgjorEgenStatus();
  }
  // hvis timer har startet og tidsgrensen er passert, sett egen status til ikke tilstede 
  // og sjekk at avstanden ikke er endret (om personen har kommet tilbake igjen f.eks.)
  else if (startTid > 0 && (millis() - startTid) >= grenseTid){
    while(avstand > grenseAvstand){
      statuskode = 4;
      settEgenStatus();
      trigUltrasoniskSensor();
    }
    avgjorEgenStatus(); 
  }
  settEgenStatus();
}

// forskjellen på settEgenStatus() og avgjorEgenStatus er at førstnevnte endrer lyset basert på statuskoden, 
// mens sistenevnte endrer statuskode (ikke lysene) basert på lesing fra reed1 switchen. 
void avgjorEgenStatus(){
  if(digitalRead(reed1) == LOW){
    statuskode = 1;
  }
  else if(digitalRead(reed2) == LOW){
    statuskode = 2;
  }
  else if (digitalRead(reed3) == LOW){
    statuskode = 3;
  }
  else{
    statuskode = 0;
  }
}

void pulsering(){
  // koden er basert på tutorialen funnet på Sparkfun sine nettsider
  /************************************************************************
  * Tittel: Pulse a LED
  * Forfatter: MikeGrusin
  * Dato: 31.10.2011
  * Lenke: https://www.sparkfun.com/tutorials/329
  /************************************************************************/
  float in, out;
  while(digitalRead(reed1) == LOW){
    for (in = 0; in < 6.283; in = in + 0.001){
      out = sin(in) * 127.5 + 127.5;
      analogWrite(r_egen, out);
      analogWrite(g_egen, 255);
      analogWrite(b_egen, 255);
    }
  }
  // nullstiller timer
  startTidOpptatt = 0;
}

// metoden for å simulere at andre teammedlemmer endrer sin status.
// metoden itererer gjennom et array med tallverdier (som representerer farger) og for hver gang et RGB lys skifter farge
// blir forrigeRGB variabelen endret. På denne måten varierer det både hvilken RGB led som bytter farge, og hvilken farge
// det byttes til

void skiftFarge(){
  int fargemonster[] = {
    // kolonne 1 er rgb sor sitt lysmønster, kol 2 = rgb vest, kol 3 = rgb nord
    1, 3, 4, 
    2, 1, 3, 
    3, 3, 2, 
    4, 1, 3};

    // hvis forrigeRGB var nord, bytt farge på sør og oppdatert forrigeRGB til sør
    if(forrigeRGB == 1){
      forrigeRGB = 2;
      skiftFargeVestSor(fargemonster[teller], r_sor, g_sor, b_sor);
      teller++;
    }
    // hvis forrigeRGB var sør, bytt farge på vest og oppdatert forrigeRGB til vest
    else if(forrigeRGB == 2){
      forrigeRGB = 3;
      skiftFargeVestSor(fargemonster[teller], r_vest, g_vest, b_vest);
      teller++;
    }
    // hvis forrigeRGB var vest, bytt farge på nord og oppdatert forrigeRGB til nord
    else if(forrigeRGB == 3){
      forrigeRGB = 1;
      skiftFargeNord(fargemonster[teller], r_nord, g_nord, b_nord);
      teller++;
    }
  // nullstiller teller når fargermønsteret har iterert gjennom fargermonster arrayet 
  if (teller >= 11){
    teller = 0;
  }
  // oppdatert tiden når siste farge-skifte har skjedd
  tidSkiftetFarge = millis();
}

// metode endrer farge på RGB basert på fargekoden og pin nummer
// siden to av RGB-ledene (vest og sor) er "common cathode", så har de andre fargekoder enn RGB nord (se egen skiftFargeNord funsjon)
void skiftFargeVestSor(int fargekode, int r, int g, int b){
  // 1: rød    2: gul    3: blå     4: grønn
  if(fargekode == 1){
    analogWrite(r, 255);
    analogWrite(g, 0);
    analogWrite(b, 0);
  }
  else if(fargekode == 2){
    analogWrite(r, 255);
    analogWrite(g, 255);
    analogWrite(b, 0);
  }
  else if(fargekode == 3){
    analogWrite(r, 0);
    analogWrite(g, 0);
    analogWrite(b, 255);
  }
  else if(fargekode == 4){
    analogWrite(r, 0);
    analogWrite(g, 255);
    analogWrite(b, 0);
  }
}

// metode endrer farge på RGB basert på fargekoden og pin nummer
void skiftFargeNord(int fargekode, int r, int g, int b){
  // 1: rød    2: gul    3: blå     4: grønn
  if(fargekode == 1){
    analogWrite(r, 0);
    analogWrite(g, 255);
    analogWrite(b, 255);
  }
  else if(fargekode == 2){
    analogWrite(r, 0);
    analogWrite(g, 0);
    analogWrite(b, 255);
  }
  else if(fargekode == 3){
    analogWrite(r, 255);
    analogWrite(g, 255);
    analogWrite(b, 0);
  }
  else if(fargekode == 4){
    analogWrite(r, 255);
    analogWrite(g, 0);
    analogWrite(b, 255);
  }
}




















