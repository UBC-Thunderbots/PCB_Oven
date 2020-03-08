/* Functions Header */

/* Functions */
void START_BUTTON(void);
void ABORT_BUTTON(void);
void ABORT_M(void);
void printTemp(void);
void printTime(void);
double readTemp (void);
void coolingLoop(void);

/* ISR for External Interrupt 1 */
void START_BUTTON() {
  if (state == 0) {
    START = 1;
  }
  else {
    START = 0;
  }
}
/* ISR for External Interrupt 2 */
void ABORT_BUTTON() {
  if (state == 1) {
    ABORT = 1;    
  }
}

void ABORT_M() {

  PWM = 0;

  /* Display Abort message on LCD */
  lcd.setCursor(0, 0);
  lcd.print("System Aborted  ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  
  /* Warning Beep*/
  tone(BUZZER_PIN,2048,1000);
  delay(2000); // wait 2 seconds to refresh screen

  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  // Reset Variables
  state = 0;
  START = 0;
  sec = 0;
}

/*
 * Function: printTemp
 * Usage: Prints temperature to LCD
 */
void printTemp(void) {
  // Read temp if buzzer off
  if (buzzer_ms == 0) {
    Temp = readTemp();
  }
  // Display temp
  lcd.setCursor(0,1);
  lcd.print(Temp);
  lcd.print(" C  ");
}

/*
 * Function: printTime
 * Usage: Prints Runtime to LCD
 */
void printTime(void) {
    lcd.setCursor(12,1);
    lcd.print(minutes);
    lcd.print(":");
    if(seconds < 10) {
      lcd.print("0");
    }
    lcd.print(seconds);
}

/*
 * Function: readTemp
 * Usage: Reads temperature from circuit
 * Returns: Temperature as double variable
 */
double readTemp ( void ) {
  /* This is using a TMP36 Sensor - this code should be changed if using a different sensor */
  double ADC_0=0;
  double ADC_1=0;
  double TOTAL_0=0,TOTAL_1=0;
  double ADC_HJ0;
  double ADC_CJ1;
  
  for (int i = 0; i < 100; i++){
    ADC_0 = analogRead(THERMO_HJ_PIN);
    ADC_1 = analogRead(THERMO_CJ_PIN);

    TOTAL_0 += ADC_0;
    TOTAL_1 += ADC_1;
  }

  ADC_HJ0 = TOTAL_0 / 100.0;
  ADC_CJ1 = TOTAL_1 / 100.0;

    /*
  Code converted from python to arduino, simplifying tasks
  kconvert.py: Converts millivots to degrees Celcius and viceversa for K-type thermocouple.
  By Jesus Calvino-Fraga 2013-2016
  Constants and functions from http://srdata.nist.gov/its90/download/type_k.tab
  
  To use in your Python program:
  
  import kconvert
  print "For 8.15 mV with cold junction at 22 C, temperature is: ", round(kconvert.mV_to_C(8.15, 22.0),2), "C"
  
  
  Evaluate a polynomial in reverse order using Horner's Rule,
  for example: a3*x^3+a2*x^2+a1*x+a0 = ((a3*x+a2)x+a1)x+a0
  https://www.keysight.com/upload/cmc_upload/All/5306OSKR-MXD-5501-040107_2.htm?&amp&cc=CA&lc=eng
  */
  
  // 0 C to 500 C: 0 mV to 20.644 mV
  int len = 10;
  double lst[len] = {0, 25.08355, 0.078601060, -0.25031310,
               0.0831527, -0.01228034, 0.0009804036, -0.000044130300,
               0.000001057734, -0.00000001052755};

  
  // convert cold junction voltage to temperature
  double Vc = ADC_CJ1 * 5.0/1023.0;
  //double Tc = (Vc - 0.5)*100.0; // from TMP36 datasheet
  double Vh = ADC_HJ0 * 5.0/1023.0;
  
  double total = 0; 
  for ( int i=len ;i > 0 ; i--) {
      total = total * (Vh*1000+Vc*1000) + lst[i-1];
      Serial.println(total);
  }
  //double Th = Vh /(0.000041 * 100000.0/330.0); // 41uV/C is approximate temperature relation slope, R2 = 100K, R1 = 330
  
  /* Approx Temperature is Cold + Hot */
  double Temp = total;
  return Temp;
}

void coolingLoop(void) {
  printTime();
  printTemp();
  // beep
  buzzer_ms = 1000;
  tone(BUZZER_PIN,2048,1000);
  printTime();
  printTemp();
  delay(2000);// wait 2 seconds
}
