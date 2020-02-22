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
  
  for (int i = 0; i < 1000; i++){
    ADC_0 = analogRead(THERMO_HJ_PIN);
    ADC_1 = analogRead(THERMO_CJ_PIN);

    TOTAL_0 += ADC_0;
    TOTAL_1 += ADC_1;
  }

  ADC_HJ0 = TOTAL_0 / 1000.0;
  ADC_CJ1 = TOTAL_1 / 1000.0;
  
  // convert cold junction voltage to temperature
  double Vc = ADC_CJ1 * 5.0/1023.0;
  double Tc = (Vc - 0.5)*100.0; // from TMP36 datasheet
  double Vh = ADC_HJ0 * 5.0/1023.0;
  double Th = Vh /(0.000041 * 100000/330); // 41uV/C is approximate temperature relation slope, R2 = 100K, R1 = 330
  
  /* Approx Temperature is Cold + Hot */
  double Temp = Tc+Th;
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
