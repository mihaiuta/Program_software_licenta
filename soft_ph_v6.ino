#define bat_meas A5
#define ph_sense A0
#define pin_temperatura 7
const int Vref = 5;       
float valoare_ph;
float temperatura;        
float tensiune;         
float ph_tensiune;       
char caracter;       

float Offset = 0.7;         
float Av = 2.8;             
int meniu_lcd = 0;

#define nr_sample 50       
int vector_valori[nr_sample]; 
int index = 0;
unsigned long timp_achizitie = millis();
unsigned long timp_afisare = millis();

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(pin_temperatura);
DallasTemperature senzor_temperatura(&oneWire);

#include <LiquidCrystal.h>
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(2, 3);     

void setup() {        
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);   
  lcd.print("PH metru");

  Serial.begin(9600);
  Serial.println("Test");
  Bluetooth.begin(9600);
  Bluetooth.println("Test");

  senzor_temperatura.begin();
}

void citire_baterie() {
  tensiune = 0;
  for (int i = 0; i < 10; i++) {
    tensiune = tensiune + analogRead(bat_meas); 
  }
  tensiune /= 10;        
  tensiune = Vref * tensiune / 1024; 
  tensiune = tensiune *  4.03;       
}

void citire_temperatura() {
  senzor_temperatura.requestTemperatures();
  temperatura = senzor_temperatura.getTempCByIndex(0);
}

void citire_ph() {
  vector_valori[index++] = analogRead(ph_sense);
  if (index == nr_sample)index = 0;        
  ph_tensiune = mediaza_valori(vector_valori, nr_sample) * 5.0 / 1024;    
  ph_tensiune = ph_tensiune - (temperatura - 25) * 0.00138 * Av;       
  ph_tensiune = ((ph_tensiune - Offset) / Av) * 1000;   

  valoare_ph = ph_tensiune;
  if (valoare_ph < 0)valoare_ph = 7 - abs(valoare_ph / 59);      
  else valoare_ph = abs(valoare_ph / 59);
}

void afisare_lcd() {
  lcd.clear();
  if (meniu_lcd == 0) {
    lcd.setCursor(0, 0);
    lcd.print("PH:");
    lcd.print(valoare_ph);
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.print(temperatura);
  }
  else if (meniu_lcd == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor:");
    lcd.print(ph_tensiune);
    lcd.print("mV");
    lcd.setCursor(0, 1);
    lcd.print("Vbat:");
    lcd.print(tensiune);
    lcd.print("V");
  }
  else if (meniu_lcd == 2) {
    lcd.setCursor(0, 0);
    lcd.print("Offset:");
    lcd.print(Offset);
    lcd.setCursor(0, 1);
    lcd.print("Av:");
    lcd.print(Av);
  }
}

void afisare_PC() {
  Serial.print("PH:");
  Serial.println(valoare_ph);
  Serial.print("Temperatura:");
  Serial.println(temperatura);
  Serial.print("Vbat:");
  Serial.println(tensiune);
  Serial.print("Tensiune senzor:");
  Serial.print(ph_tensiune);
  Serial.println("mV");

  if (meniu_lcd == 1) {
    Serial.print("pH voltage:");
    Serial.print(ph_tensiune);
    Serial.println("mV");
    Serial.print("Vbat:");
    Serial.print(tensiune);
    Serial.println("V");
  }
  if (meniu_lcd == 2) {
    Serial.print("Offset:");
    Serial.println(Offset);
    Serial.print("Av:");
    Serial.println(Av);
  }
}

void afisare_Bluetooth() {
  Bluetooth.print("PH:");
  Bluetooth.println(valoare_ph);
  Bluetooth.print("Temperatura:");
  Bluetooth.println(temperatura);
  Bluetooth.print("Vbat:");
  Bluetooth.print(tensiune);
  Bluetooth.println("V");

  if (meniu_lcd == 1) {
    Bluetooth.print("Tensiune senzor:");
    Bluetooth.print(ph_tensiune);
    Bluetooth.println("mV");
    Bluetooth.print("Vbat:");
    Bluetooth.print(tensiune);
    Bluetooth.println("V");
  }
  if (meniu_lcd == 2) {
    Bluetooth.print("Offset:");
    Bluetooth.println(Offset);
    Bluetooth.print("Av:");
    Bluetooth.println(Av);
  }
}

double mediaza_valori(int* vector, int nr_valori) {
  int i;
  int max, min;
  double media;
  double suma = 0;
  int nr_valori_insumate=0; 

     if (vector[0] < vector[1]) {        
      min = vector[0]; max = vector[1];     
    }
    else {
      min = vector[1]; max = vector[0];
    }
    
    for (i = 2; i < nr_valori; i++) {   
      if (vector[i] < min) {
        min = vector[i];
      } else {
        if (vector[i] > max) {
          max = vector[i];
        } else {           
          suma += vector[i];    
          nr_valori_insumate++;       
        }
      }
    }
    media = suma /nr_valori_insumate;
  return media;
}

void loop() {       

  if (Serial.available()) {   
    caracter = Serial.read();   
  }
  if (Bluetooth.available()) {   
    caracter = Bluetooth.read();   
  }

  if (caracter == 'm') {        
    caracter = "";            
    meniu_lcd++;     
    if (meniu_lcd > 2)meniu_lcd = 0;
    afisare_lcd();
    Bluetooth.println("Schimbare mod afisare");
  }

  if (caracter == 'o') {        
    caracter = "";            
    Offset += 0.5;
    afisare_lcd();
  }

  if (caracter == 'i') {        
    caracter = "";            
    Offset -= 0.5;
    afisare_lcd();
  }

  if (millis() - timp_achizitie > 20) {           
    citire_ph();
    timp_achizitie = millis();
  }

  if (millis() - timp_afisare > 1000)  {       
    citire_baterie();
    citire_temperatura();
    citire_ph();
    afisare_lcd();
    afisare_PC();
    afisare_Bluetooth();
    timp_afisare = millis();
  }

}
