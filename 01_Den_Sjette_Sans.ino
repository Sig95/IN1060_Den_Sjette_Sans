
const int triggerPin = 50; // ulytasonisk sensor, trigger pin. gul ledning
const int echoPin = 52;    // ultrasonisk sensor, echo pin. rød/oransje ledning
long etappeTid;           // tiden det tar for den ultrasoniske bølgen å gå fra trigger til echo
int avstand;              // avstand (i cm) US sensor måler

// reed switch variabler settes til pins med interrupt funksjon (pin: 2, 3, 18, 19, 20, 21 på arduino mega)
const int reed1 = 20;       
const int reed2 = 21;       
const int reed3 = 2; 

// egenRGB
const int r_egen = 4;
const int g_egen = 5;
const int b_egen = 6;
int statuskode;                 // endres når reed switchene trigges,  1:opptatt   2:samarbeid   3:pause  

const unsigned long opptattPaaminnelse = 1800000; //Hvis brukeren har på rød i 30 min sammenhengende, vil hen få en påminnelse om dette i form av pulsering av LED 
unsigned long startTidOpptatt = 0; // tidspunkt når bruker plasserer magnet på opptatt-status
bool reed1_state = true; // holder styring på om opptatt reed har magnet på seg eller ikke

// grense-variabler
const unsigned long grenseTid = 900 * 1000; // tid før system automatisk setter status "ikke tilstede". Angi i sekunder * 1000 (for å gjøre om til millisekunder)
const int grenseAvstand = 80;   // avstand i cm. Om ultrasonisk sensor detekterer avstand større enn dette, settes eget rgb lys til hvitt

// variabler for tid
unsigned long startTid;

// anode
const int r_nord = A2;
const int g_nord = A1;
const int b_nord = A0;

// katode
const int r_sor = A3;
const int g_sor = A5;
const int b_sor = A4;

// katode
const int r_vest = A8;
const int g_vest = A6;
const int b_vest = A7;

unsigned long tidSkiftetFarge = millis();
const unsigned long rgb_grensetid = 8 * 1000;

int teller = 0;
int forrigeRGB = 1;
bool bytte = true; // tid før rgb lys bytter farge

void setup() {
  // sjekker om magneten er plassert på en reed switch. Hvis ja, så settes statuskode til korresponderende tall 
  avgjorEgenStatus();

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(reed1, INPUT_PULLUP);
  pinMode(reed2, INPUT_PULLUP);
  pinMode(reed3, INPUT_PULLUP);
  // om en av reed switchene aktiveres, vil programmet automatisk hoppe til korresponderende funksjon og utføre dette
  attachInterrupt(digitalPinToInterrupt(reed1), settOpptatt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(reed2), settSamarbeid, CHANGE);
  attachInterrupt(digitalPinToInterrupt(reed3), settPause, CHANGE);

  pinMode(r_egen, OUTPUT);
  pinMode(g_egen, OUTPUT);
  pinMode(b_egen, OUTPUT);
  
  const int r_nord = A2;
const int g_nord = A1;
const int b_nord = A0;

// katode
const int r_sor = A3;
const int g_sor = A5;
const int b_sor = A4;

// katode
const int r_vest = A8;
const int g_vest = A6;
const int b_vest = A7;

}

void loop() {
  settEgenStatus();
  trigUltrasoniskSensor();
  sjekkTilstedevaerelse();


  //sjekker om bruker har hatt status opptatt X antall tid. Hvis ja, starter pulsering.
    if ((millis() - startTidOpptatt) > opptattPaaminnelse){
      pulsering();
    }
  
  
  if((millis() - tidSkiftetFarge) > rgb_grensetid){
    if(bytte){
      analogWrite(r_nord, 255);
      analogWrite(g_nord, 0);
      analogWrite(b_nord, 255);
      bytte = false;
      tidSkiftetFarge = millis();
    }
    else{
      analogWrite(r_nord, 0);
      analogWrite(g_nord, 255);
      analogWrite(b_nord, 255);
      bytte = true;
      tidSkiftetFarge = millis();
      
    }

    
  } 
  
}


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

void settEgenStatus(){
  // opptatt - rød
  if(statuskode == 1){      
    analogWrite(r_egen, 0);
    analogWrite(g_egen, 255);
    analogWrite(b_egen, 255);

   



    if(reed1_state && startTidOpptatt == 0){
      startTidOpptatt = millis();
      reed1_state = false;
    }
    else{
      reed1_state = true;
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
  else if (statuskode == 0){
    analogWrite(r_egen, 255);
    analogWrite(g_egen, 0);
    analogWrite(b_egen, 255);

    
    //nullstiller startTidOpptatt
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
  //US måler lengde
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
  // hvis timer har startet og tidsgrensen er passert, skru på LED
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

void skiftFarge(){
  // rgb led variabel    1 = nord    2 = sor     3=vest
  int fargemonster[] = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4};
  // global variabel int teller = 0;
  while(teller < 11){
    
    if(forrigeRGB == 1){
      forrigeRGB = 2;
      
      skiftFargeVestSor(fargemonster[teller], r_sor, g_sor, b_sor);
      teller++;
    }
    else if(forrigeRGB == 2){
      forrigeRGB = 3;
      skiftFargeVestSor(fargemonster[teller], r_vest, g_vest, b_vest);
      teller++;
    }
    else if(forrigeRGB == 3){
      forrigeRGB = 1;
      skiftFargeNord(fargemonster[teller], r_nord, g_nord, b_nord);
      teller++;
    }
  }
  teller = 0;
  tidSkiftetFarge = millis();
}


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




















