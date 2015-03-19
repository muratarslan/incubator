
/*=======================================================================
                            KUVEZ V1.0
=======================================================================*/


#include <LiquidCrystal.h> // LCD library


/*=======================================================================
                   ADC DEFINING 
=======================================================================*/
#define DS1307_ADDRESS 0x68
byte zero = 0x00; //workaround for issue #527


/*=======================================================================
                   BUTTON ADC DEFINING
=======================================================================*/
#define BUTTON_ADC_PIN            A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         2  // D2 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            64  // up
#define DOWN_10BIT_ADC          158  // down
#define LEFT_10BIT_ADC          288  // left
#define SELECT_10BIT_ADC        533  // select
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window


/*=======================================================================
                     BUTTON DEFINING 
=======================================================================*/
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 


/*=======================================================================
                   BACKLIGHT DEFINING
=======================================================================*/
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }


/*=======================================================================
                      BUTTON VARIABLES
=======================================================================*/
byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events


/*=======================================================================
                         TIMER VARIABLES
=======================================================================*/
int time[4]; // Time array
int time_fake[4]; // Fake Time Array

int cur = 3; // Cursor Variables
int start = 0; // Start- stop conrtol variable


/*=======================================================================
                   TEMPERATURE LDR & SOUND VARIABLES
=======================================================================*/
int temp_ort[10];
int temp = 0;


int ldr_ort[10];
int ldr = 0;

int sound_ort[10];
int sound = 0;


/*=======================================================================
                       RELAY & FAN VARIABLES
=======================================================================*/
int relay = 13;
int fan = 12;

/*=======================================================================
                              LCD SETUP
=======================================================================*/
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );    //( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )

void setup()
{
  Serial.begin(9600);
 
  
  //button adc input
  pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
  //lcd backlight control
  digitalWrite( LCD_BACKLIGHT_PIN, HIGH );  //backlight control pin D3 is high (on)
  pinMode( LCD_BACKLIGHT_PIN, OUTPUT );     //D3 is an output
  
  // Relay & Fan pin output
  pinMode(relay, OUTPUT);
  pinMode(fan, OUTPUT);
  digitalWrite(relay, LOW);
  digitalWrite(fan, LOW);
  
  //set up the LCD number of columns and rows: 
  lcd.begin( 16, 2 );
  lcd.setCursor(0, 0);
  lcd.print("Yusuf Incekalan");
  lcd.setCursor(0, 1);
  lcd.print("Suleyman Oksuz");
  delay(2000);
  LCD_BACKLIGHT_OFF();
}




/*=======================================================================
                             MAIN LOOP 
=======================================================================*/
void loop()
{
  byte button;
  byte timestamp;

  LCD_BACKLIGHT_OFF();
  
  //get the latest button pressed, also the buttonJustPressed, buttonJustReleased flags
  button = ReadButtons();
      
 // Control Buttons  
  switch (button) {
  case 0:    // your hand is on the sensor
    lcd.setCursor(0, 1);// cursor bir alt satıra yönlendiriliyor
    if(start % 2 == 1){
    print_time_fake();}
    if(start % 2 == 0){
    print_time();}
    break;
  case 1:    // your hand is close to the sensor
    lcd.setCursor(0, 1);// cursor bir alt satıra yönlendiriliyor
    cur++;
    if (cur < 0)
     cur = 3;
    else if (cur > 3)
     cur = 0;  
    print_time();
    curs();
    break;
  case 2:    // your hand is a few inches from the sensor
    lcd.setCursor(0, 1);// cursor bir alt satıra yönlendiriliyor
    time[cur]++;
    if (time[cur] < 0)
     time[cur] = 0; 
    print_time();
    LCD_BACKLIGHT_ON();
    break;
  case 3:    // your hand is nowhere near the sensor
    lcd.setCursor(0, 1);// cursor bir alt satıra yönlendiriliyor
    time[cur]--;
    if (time[cur] < 0)
     time[cur] = 0; 
    print_time();
    LCD_BACKLIGHT_ON();
    break;
  case 4:    // your hand is nowhere near the sensor
    lcd.setCursor(0, 1);// cursor bir alt satıra yönlendiriliyor
    cur--;
    if (cur < 0)
     cur = 3;
    else if (cur > 3)
     cur = 0; 
    print_time();
    curs();
    break;
  case 5:    // your hand is nowhere near the sensor
    //lcd.clear();
    lcd.setCursor(0, 1);// cursor bir alt satıra yönlendiriliyor
    LCD_BACKLIGHT_ON();
    if(start % 2 == 0){
      lcd.print ("      START    ");
      lcd.print(" ");
      delay(1000);
    }
      
    else if(start % 2 == 1){
    lcd.print ("      STOP    ");
    delay(1000);
   }
  
    start++;
    break;
  } 
      
  //clear the buttonJustPressed or buttonJustReleased flags, they've already done their job now.
  if( buttonJustPressed )
    buttonJustPressed = false;
  if( buttonJustReleased )
    buttonJustReleased = false;
 

temper_ldr_sound();
times();
}


/*=======================================================================
                   TEMPERATURE LDR & SOUND 
=======================================================================*/
void temper_ldr_sound(){
 for(int i=0; i<10; i++){ // Sicaklik Sensoru Ortalamasi
   temp_ort[i] = analogRead(A2);
   temp = temp_ort[i] + temp;
   delay(20);
 }   
 
  for(int i=0; i<10; i++){ // LDR Sensoru Ortalamasi
   ldr_ort[i] = analogRead(A1);
   ldr = ldr_ort[i] + ldr;
   delay(20);
 }   
 
  for(int i=0; i<10; i++){ // Ses Sensoru Ortalamasi
   sound_ort[i] = analogRead(A5);
   sound = sound_ort[i] + sound;
   delay(20);
 }   
 
 temp = temp/25;
 sound = sound / 10;
 ldr = ldr / 10;
 print_temp();
}



/*=======================================================================
                   PRINT TIME FUNCTION
=======================================================================*/
void print_time(){
  lcd.setCursor(0,1);
  lcd.print ("Timer: ");
  lcd.print (time[0]);
  lcd.print (time[1]);
  lcd.print (":");
  lcd.print (time[2]);
  lcd.print (time[3]);
  lcd.print ("    ");
}



/*=======================================================================
                   PRINT TIME FAKE FUNCTION
=======================================================================*/
void print_time_fake(){
      
  int timer = 0;
  byte buttons;

  for(int i=0; i<4; i++){
        time_fake[i] = time[i];
  }
  
  while(((time_fake[2]*10) + time_fake[3])!=timer){
    int tt = ((time_fake[2]*10) + time_fake[3])-timer;
    buttons = ReadButtons();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("       ");
    lcd.print (tt);
    lcd.setCursor(0,1);
    lcd.print ("   Lights On!");
    digitalWrite(relay, HIGH);
    if(buttons == 5)break;
    delay(1000);
    timer++;
  }
  start++;
  lcd.clear();
  lcd.setCursor(0,1);
  LCD_BACKLIGHT_ON();
  lcd.print ("   Lights Off!          ");
  digitalWrite(relay, LOW);
  delay(1000);
 }
  
 



/*=======================================================================
                   PRINT TEMPERATURE FUNCTION
=======================================================================*/
void print_temp(){
  lcd.clear();
  lcd.print ("T:");
  lcd.print (temp);
  lcd.print ("C");
  lcd.print (" ");
  lcd.print ("L:");
  lcd.print (ldr);
  lcd.print (" ");
  lcd.print ("S:");
  lcd.print (sound);
}


/*=======================================================================
                   TIME CONTROL FUNCTION
=======================================================================*/
void times(){
  
  if(time[0] > 9)
    time[0] = 0;
  if(time[1] > 9)
    time[1] = 0;
  if(time[2] > 5){
    time[2] = 0;
    time[1]=time[1]+1;}
  if(time[3] > 9){
    time[3] = 0;
    time[2]=time[2]+1;}
    
}


/*=======================================================================
                   CURSOR FUNCTION
=======================================================================*/
void curs(){
    
    if(cur == 0){
    lcd.setCursor(0, 1);
    lcd.print ("Timer: ");
    lcd.print ("_");
    lcd.print (time[1]);
    lcd.print (":");
    lcd.print (time[2]);
    lcd.print (time[3]);
    lcd.print ("    ");
    LCD_BACKLIGHT_ON();}
    
    else if(cur == 1){
    lcd.setCursor(0, 1);
    lcd.print ("Timer: ");
    lcd.print (time[0]);
    lcd.print ("_");
    lcd.print (":");
    lcd.print (time[2]);
    lcd.print (time[3]);
    lcd.print ("    ");
    LCD_BACKLIGHT_ON();}
    
    else if(cur == 2){
    lcd.setCursor(0, 1);
    lcd.print ("Timer: ");  
    lcd.print (time[0]);
    lcd.print (time[1]);
    lcd.print (":");
    lcd.print ("_");
    lcd.print (time[3]);
    lcd.print ("    ");
    LCD_BACKLIGHT_ON();}
    
    else if(cur == 3){
    lcd.setCursor(0, 1);
    lcd.print ("Timer: ");  
    lcd.print (time[0]);
    lcd.print (time[1]);
    lcd.print (":");
    lcd.print (time[2]);
    lcd.print ("_");
    lcd.print ("    ");
    LCD_BACKLIGHT_ON();}
}





/*=======================================================================
                        BUTTON FUNCTION 
=======================================================================*/
byte ReadButtons()
{
  unsigned int buttonVoltage;
  byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn

  //read the button ADC pin voltage
  buttonVoltage = analogRead( BUTTON_ADC_PIN );
  //sense if the voltage falls within valid voltage windows
  if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_RIGHT;
  }
  else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_UP;
  }
  else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_DOWN;
  }
  else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_LEFT;
  }
  else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_SELECT;
  }
  //handle button flags for just pressed and just released events
  if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
  {
    //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
    //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
    buttonJustPressed  = true;
    buttonJustReleased = false;
  }
  if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
  {
    buttonJustPressed  = false;
    buttonJustReleased = true;
  }

  //save the latest button value, for change event detection next time round
  buttonWas = button;

  return( button );
}
