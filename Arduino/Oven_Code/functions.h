/* Functions Header */
void initialiseTimer1(void);
void START_BUTTON(void);
void ABORT_BUTTON(void);
void ABORT_MANUAL(void);
void printTemp(void);
void printTime(void);
double readTemp (void);
double C_to_mV(double tempC);
double mV_to_C(double mVolts, double ColdJunctionTemp);
double PolyEval(double lst[],double x);
void coolingLoop(void);
int receiveParameter(void);


void initialiseTimer1(void) {
/* Set up timer1 */
  cli();           // disable global interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 65473; // 65536-16MHz/(256*1KHz)  || Using 1KHz Frequency for PWM
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  sei();             // enable global interrupts
}

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

void ABORT_MANUAL() {

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
  state_seconds = 0;
}

/*
 * Function: printTemp
 * Usage: Prints temperature to LCD
 */
void printTemp(void) {
  // Display temp
  lcd.setCursor(0,1);
  lcd.print(Temp,1);
  lcd.print((char)0xDF);
  lcd.print("C     ");
  if(Temp < 100) {
    lcd.print(" ");
  }
  if(state == 0) {
    lcd.print("    ");
  }
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
  double ADC_HJ0 = analogRead(THERMO_HJ_PIN);
  double ADC_CJ1 = analogRead(THERMO_CJ_PIN);
  /*
    for (int i = 0; i < 100; i++){
      ADC_0 = analogRead(THERMO_HJ_PIN);
      ADC_1 = analogRead(THERMO_CJ_PIN);

      TOTAL_0 += ADC_0;
      TOTAL_1 += ADC_1;

    }

    ADC_HJ0 = TOTAL_0 / (100.0);
    ADC_CJ1 = TOTAL_1 / (100.0);
  */

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

  // convert cold junction voltage to temperature
  double Vc = ADC_CJ1 * 5.0/1023.0;
  double Tc = (Vc - 0.5)*100.0; // from TMP36 datasheet
  double Vh = ADC_HJ0 * 5.0/1023.0;
  double Temp = mV_to_C(Vh*1000/(98550.0/325.45),Tc);
  //Serial.println(Vh*1000);
  //double Th = Vh /(0.000041 * 100000.0/330.0); // 41uV/C is approximate temperature relation slope, R2 = 100K, R1 = 330
  /* Approx Temperature is Cold + Hot */
  return Temp;
}

//   0 C to 500 C: 0 mV to 20.644 mV
double mV_to_C_2[] = {0.0000000E+00, 2.5083550E+01, 7.8601060E-02, -2.5031310E-01,
             8.3152700E-02, -1.2280340E-02, 9.8040360E-04, -4.4130300E-05,
             1.0577340E-06, -1.0527550E-08};
//   0 C to 1372 C
double C_to_mV_2[] = { -0.176004136860E-1, 0.389212049750E-1, 0.185587700320E-4, -0.994575928740E-7,
              0.318409457190E-9, -0.560728448890E-12, 0.560750590590E-15, -0.320207200030E-18,
              0.971511471520E-22, -0.121047212750E-25};
double a[3] = {0.1185976, -0.118343200000E-3, 0.126968600000E3};

double C_to_mV(double tempC) {
  return PolyEval(C_to_mV_2, tempC) + a[0] * exp(a[1] * (tempC - a[2]) * (tempC - a[2]));
}
double mV_to_C(double mVolts, double ColdJunctionTemp){
  double total = mVolts + C_to_mV(ColdJunctionTemp);
  return PolyEval(mV_to_C_2,total);
}
        
double PolyEval(double lst[],double x) {
  double total = 0;
  for ( int i=10 ;i > 0 ; i--) {
      total = total * x + lst[i-1];
  }
  return total;
}

void coolingLoop(void) {
  printTime();
  printTemp();
  // beep
  tone(BUZZER_PIN,2048,1000);
  printTime();
  printTemp();
  delay(2000);// wait 2 seconds
}

// Description:   1. Receives a thermal profile parameter from Python via the serial port
//                2. Echoes the received value back to Python to ensure that the correct data has been received
//                3. Provides user feedback via the buzzer
// Parameters:    -
// Returns:       The value of the received thermal profile parameter
int receiveParameter(void)
{
  int value = 0;
  
  while(value == 0)
  {
    if(Serial.available() > 0)                                                // Check if data has arrived in the serial receive buffer
    {
      value = Serial.parseInt();                                              // Read data from the buffer and convert to an integer
      Serial.print(value);                                                    // Echo received value back to Python
      Serial.print("\n");

    }
    Temp = readTemp();
    lcd.setCursor(0,1);
    lcd.print(Temp,0);
    lcd.print((char)0xDF);
    lcd.print("C");
  }
  tone(BUZZER_PIN,2048,1000);

  return value;
}