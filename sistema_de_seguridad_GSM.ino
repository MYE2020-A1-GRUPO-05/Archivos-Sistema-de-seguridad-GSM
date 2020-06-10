/******************************************************************************
*                                  Proyecto:                                  *
*                           SISTEMA DE SEGURIDAD GSM                          *
*******************************************************************************
Este proyecto permite enviar mensajes de texto SMS de alertas por violación o 
acceso no autorizado hacia un usuario establecido o usuario master, recibir
mensajes de solicitud de estado del sistema por cuatro usuarios registrados en
el sistema, recibir códigos de activación y desactivación del sistema, y enviar
estados.
******************************************************************************/

/*Circuito LCD:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 10
 * LCD D5 pin to digital pin 6
 * LCD D6 pin to digital pin 5
 * LCD D7 pin to digital pin 4
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)*/
 
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 10, 6, 5, 4);

//  Numeros celulares registrados en el sistema.
String 	Number_One    = "+50379368897";   // Número master o Cliente master
String 	Number_Two    = "+50385478547";   // Cliente 2
String 	Number_Three  = "+50351235458";   // Cliente 3
String 	Number_Four   = "+50314522542";   // Cliente 4

String  Comment_Room_1 = "";
String  Comment_Room_2 = "";
String  Comment_Lamp = "";
String  keys = "";
String  Cell_Number = "";
int	Find_Chars = 0;
char    incoming_char;
char    Digits;
char 	Character1;
char	Character2;
char 	Character3;
char 	Character4;
char 	Character5;
char 	Character6;
char 	Character7;
char 	country_code_1;
char 	country_code_2;
char 	country_code_3;
boolean Country_code = false;

// Declaración de pines digitales
int 	Alarm	 = 2;  
int 	Lamp	 = 3;
int 	Door_1	 = 48;
int 	Door_2	 = 49;
int 	Win_1 	 = 50;
int 	Win_2	 = 51;
int 	Door_3	 = 52;
int 	Win_3	 = 53;
int 	POWERKEY = 9;

// Declaración de puerto analogico 0 para un sensor fotoresistivo.
int 	PhotoRes = A0;

boolean Presence = false;
boolean State_Room_1 = false;
boolean State_Room_2 = false;
boolean State_Lamp = false;
int 	State_Door_1;
int 	State_Door_2;
int 	State_Win_1;
int 	State_Win_2;
int 	State_Door_3;
int 	State_Win_3;

int 	Count_Room_1 = 0;
int 	Count_Room_2 = 0;
int 	Count_Lig = 0;

byte customChar[8] = {  // creación del caracter °
  B01110,
  B01010,
  B01110,
  B00000,
  B01110,
  B00000,
  B00000,
  B00000
};

void setup(){
  pinMode(Alarm,   OUTPUT);
  pinMode(Lamp,    OUTPUT);
  pinMode(Door_1,  INPUT);
  pinMode(Door_2,  INPUT);
  pinMode(Win_1,   INPUT);
  pinMode(Win_2,   INPUT);
  pinMode(Door_3,  INPUT);
  pinMode(Win_3,   INPUT);
  pinMode(POWERKEY,OUTPUT);
  Serial.begin(19200);
  lcd.begin(16, 2);
  lcd.createChar(0, customChar);
  lcd.setCursor(3, 0);
  lcd.print("BIENVENIDO");
  SIM900_AUTO_POWER();  // encendido automático del módulo SIM900.
  lcd.setCursor(0, 1);
  lcd.print("INICIANDO MODULO");
  delay(1000);
  Serial.print("AT+CMGF=1\r"); // configura SMS en modo de texto.
  delay(100);
  Serial.print("AT+CNMI=2,2,0,0,0\r");  /* Transmisión de mensaje directamente
                                          al puerto serial del dispositivo. */
  delay(100);
  Send_initial_message();
  Serial.println("AT+CMGD=1,4"); // eliminar todos los mensajes
  Serial.println("AT+CMGL=\"ALL\",0 ");
  Comment_Room_1 = "Habitacion 1 Desactivado.";
  Comment_Room_2 = "Habitacion 2 Desactivado.";
  Comment_Lamp   = "Luces Apagadas.";
  lcd.clear();
}

void loop(){
  lcd.setCursor(0, 0);
  lcd.print("SECURITY SYSTEM");  
  if(Cell_Number != ""){  //imprimo el número telefónico en la LCD.
    lcd.setCursor(0, 1);
    lcd.print("N");
    lcd.write(byte(0));
    lcd.print(":");
    lcd.print(Cell_Number);
  }
  
  int sensor = analogRead(PhotoRes);
  int Light_level = map(sensor, 0, 1023, 0, 255); // Mapeamos datos de 0 a 255
  
  if (Serial.available() > 0){
    Receive_Messages();
  }
  
  //Asignamos el número telefónico remitente y enviamos una alerta.
  if  ((Country_code == true) &&  ((Cell_Number == Number_One) ||
  (Cell_Number == Number_Two) || (Cell_Number == Number_Three) || 
  (Cell_Number == Number_Four))){
    Cell_Number = Cell_Number;
  }
  
  // leemos las contraseñas
  if(keys == "Act ha1"){       // activa cuarto 1
    keys = "";
    State_Room_1 = true;
    Comment_Room_1 = "Habitacion 1 Activado.";
  }
  else if(keys == "Des ha1"){  // desactiva cuarto 1
    keys = "";
    State_Room_1 = false;
    Comment_Room_1 = "Habitacion 1 Desactivado.";
  }
  else if(keys == "Act ha2"){  // activa cuarto 2
    State_Room_2 = true;
    Comment_Room_2 = "Habitacion 2 Activado.";
  }
  else if(keys == "Des ha2"){  // desactiva cuarto 2
    keys = "";
    State_Room_2 = false;
    Comment_Room_2 = "Habitacion 2 Desactivado.";
  }
  else if(keys == "Act luc"){  // activa luces
    keys = "";
    State_Lamp = true;
    digitalWrite(Lamp, HIGH);
    Comment_Lamp = "Luces Encendidas.";
  }
  else if(keys == "Des luc"){  // desactiva luces
    keys = "";
    State_Lamp = false;
    digitalWrite(Lamp, LOW);
    Comment_Lamp = "Luces Apagadas.";
  }
  else if(keys == "Des ala"){  // desactiva alarma
    keys = "";
    digitalWrite(Alarm, LOW);
  }
  else if(keys == "Estados"){  // Petición "Estados"
    keys = "";
    Send_State_System();
  }
  else;
  
  // Alarma de puertas y ventanas del cuarto 1.
  if(State_Room_1 == true){
    // Sistema de seguridad de puertas y ventanas activadas.
    State_Door_1  = digitalRead(Door_1);
    State_Door_2  = digitalRead(Door_2);
    State_Win_1   = digitalRead(Win_1);
    State_Win_2	  = digitalRead(Win_2);
    if((State_Door_1 == 1) || (State_Door_2 == 1) || 
        (State_Win_1 == 1) || (State_Win_2 == 1)){
      Count_Room_1 ++;
      digitalWrite(Alarm, HIGH);
      if(Count_Room_1 == 1){
        Send_Security_Alert();
      }
    }
    else Count_Room_1 = 0;
    // Alarma de luces.
    if(State_Lamp == false){    // Luces apagadas
      // Si durante el proceso de pagado de luces se encienden, activa alarma.
      if(Light_level >= 200){
        Count_Lig ++;
        digitalWrite(Alarm, HIGH);
        Comment_Lamp = "Luces Encenidas.";
        if(Count_Lig == 1){
          Send_Security_Alert();
        }
      }
      else Count_Lig = 0;
    }
  }
  else{
    State_Door_1  = 0;
    State_Door_2  = 0;
    State_Win_1   = 0;
    State_Win_2   = 0;
  }
  
  // Alarma de puerta y ventana del cuarto 2.
  if(State_Room_2 == true){
    // Sistema de seguridad de puerta y ventana activadas.
    State_Door_3  = digitalRead(Door_3);
    State_Win_3   = digitalRead(Win_3);
    // Si se abren la puerta o ventana, se activa la alarma.
    if((State_Door_3 == 1) || (State_Win_3 == 1)){
      Count_Room_2 ++;
      digitalWrite(Alarm, HIGH);
      if(Count_Room_2 == 1){
        Send_Security_Alert();
      }
    }
    else Count_Room_2 = 0;
  }
  else{
    State_Door_3 = 0;
    State_Win_3  = 0;
  }
}

///////////////////////////FUNSIONES////////////////////////////////////////

// Encendido automático del módulo SIM900
void SIM900_AUTO_POWER(){
  digitalWrite(POWERKEY, HIGH);
  delay(1000);
  digitalWrite(POWERKEY, LOW);
  delay(7000);
}

//////////////  Salida de mensajes de bienvenida  /////////////////////
void Send_initial_message(){
  Serial.println("AT + CMGS = \"" + Number_One + "\"");
  delay(100);
  Serial.print("BIENBENIDOS!");
  Serial.print('\n');
  delay(100);
  Serial.print("Iniciando Sistema de Seguridad...");
  Serial.print('\n');
  delay(100);
  Serial.write((char)26); //ctrl+z
  delay(1000);
}

//////////////  Salida de mensajes de estados  /////////////////////
void Send_State_System(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENVIANDO...");
  lcd.setCursor(0, 1);
  lcd.print("SOLICITUD ESTADO");
  delay(3000);
  Serial.println("AT + CMGS = \"" + Cell_Number + "\"");
  delay(100);
  Serial.print(Comment_Room_1);
  Serial.print('\n');
  delay(100);
  Serial.print(Comment_Room_2);
  Serial.print('\n');
  delay(100);
  Serial.print(Comment_Lamp);
  Serial.print('\n');
  delay(100);
  Serial.write((char)26); //ctrl+z
  delay(1000);
}

//////////////  Salida de mensajes de alertas  /////////////////////
void Send_Security_Alert(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALERTA!!!");
  lcd.setCursor(0, 1);
  lcd.print("SISTEMA VIOLADO");
  delay(3000);
  Serial.println("AT + CMGS = \"" + Number_One + "\"");
  delay(100);
  Serial.print("ALERTA!!!");
  Serial.print('\n');
  delay(100);
  Serial.print("Violacion al Sistema de Seguridad.");
  Serial.print('\n');
  delay(100);
  Serial.write((char)26); //ctrl+z
  delay(1000);
}

//////////////  Llegada mensajes de texto  /////////////////////
void Receive_Messages(){
  incoming_char = Serial.read();
  if((Find_Chars == 0) && (incoming_char == 'C')){
    Find_Chars = 1;
    Cell_Number = "";
    keys = "";
  }
  if((Find_Chars == 1) && (incoming_char == 'M')){
    Find_Chars = 2;
  }
  if((Find_Chars == 2) && (incoming_char == 'T')){
    Find_Chars = 3;
  }
  if((Find_Chars == 3) && (incoming_char == ':')){
    Find_Chars = 4;
  }
  if((Find_Chars == 4) && (incoming_char == ' ')){
    Find_Chars = 5;
  }
  if((Find_Chars == 5) && (incoming_char == '"')){
    Find_Chars = 6;
    for(int i = 0; i <= 13; i++){
      Digits = Serial.read();
      if (Digits == '"') break;
      else Cell_Number += Digits;  // Obtengo todo el número telefónico.
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RECIBIENDO");
    lcd.setCursor(0, 1);
    lcd.print("MENSAJE...");
    delay(3000);
    country_code_1 = (char)Cell_Number[1];
    country_code_2 = (char)Cell_Number[2];
    country_code_3 = (char)Cell_Number[3];
    if ((country_code_1 == '5') && (country_code_2 == '9') && 
        (country_code_3 == '3')) Country_code = true; // código país.
    else Country_code = false;
  }
  // la clave va seguido del caracter Numeral "\n".
  if ((Find_Chars == 6) && (incoming_char == '\n')){
    Find_Chars = 0;
    // Todas las claves están comprendidas en un tamaño de 7 caracteres.
    Character1 = Serial.read();
    Character2 = Serial.read();
    Character3 = Serial.read();
    Character4 = Serial.read();
    Character5 = Serial.read();
    Character6 = Serial.read();
    Character7 = Serial.read();
    keys += Character1;
    keys += Character2;
    keys += Character3;
    keys += Character4;
    keys += Character5;
    keys += Character6;
    keys += Character7;
  }
  delay(100);
}