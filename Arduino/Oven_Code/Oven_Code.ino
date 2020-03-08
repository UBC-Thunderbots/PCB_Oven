#include "variables.h"
#include "functions.h"

void setup() {
  /* Use Supply voltage for measuring the ADC value */
  //analogReference(EXTERNAL);
  
  /* LDC Set up as 16 columns and 2 rows */
  lcd.begin(16, 2);

  /* Start the serial interface */
  Serial.begin(9600);

  /* Set up Output pins */
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(PULSE_PIN,OUTPUT);

  /* Set up Input pins */
  pinMode(START_KEY, INPUT_PULLUP);
  pinMode(ABORT_KEY,INPUT_PULLUP);

  /* Set pins off */
  digitalWrite(PULSE_PIN,LOW);
  digitalWrite(BUZZER_PIN,LOW);

  /* Set up external interrupts */
  attachInterrupt(digitalPinToInterrupt(START_KEY), START_BUTTON, FALLING);
  attachInterrupt(digitalPinToInterrupt(ABORT_KEY),ABORT_BUTTON,FALLING);

  /* Set up timer1 */
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 65473; // 65536-16MHz/(256*1KHz)  || Using 1KHz Frequency for PWM
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

/* Interrupt Service Routine for Timer 1 */
ISR(TIMER1_OVF_vect)        // interrupt service routine for complement oven
{
  TCNT1 = timer1_counter;   // preload timer

  pulse_on = 1000 - PWM*10;
  if ( Count1ms >= 1000){
    /* send temperature to serial port and LCD every second */
    printTemp();
    Serial.println(Temp);
    
    sec += 1;
    seconds += 1;
    Count1ms = 0;
    if(seconds == 60) {
      minutes += 1;
      seconds = 0;
    }
    PULSE = 0;

  }
  // second hasnt passed
  else if (Count1ms >= pulse_on) {
    PULSE = 1;
  }

  //increment the ms counter
  Count1ms += 1;

  if (buzzer_ms != 0) {
    buzzer_ms--;
  }
}

void loop() { 
  /* Only read temperature when buzzer is off */
  if (buzzer_ms == 0) {
    Temp = readTemp();
  }

  digitalWrite(PULSE_PIN,PULSE);

  /* Display State on display as long as there is no abort */
  if (ABORT == 0 ) {
    lcd.setCursor(0, 0);
    lcd.print("S:");
    lcd.print(state);
     /*if (millis() - lastmillis >= 1000) {
       printTemp();
       lastmillis = millis();
     }*/
  }
  else {
    // abort command
    ABORT_M();
    // reset abort variable
    ABORT = 0;
  }
  
  /* Print Controller runtime since start */
  if (state != 0) {
    printTime();
  }
 
  // FSM
  switch (state) {
    case 0 : 
      PWM = 0;

      // wait until start key is pressed to start the reflow cycle
      if (START == 1) {
        /* Variables set to default */
        sec = 0;
        seconds = 0;
        minutes = 0;
        state = 1;
        //Serial.println(0); // Set temp to zero temporarily to get a Tickmark in the graph for the start time
        
        /* Display next State message */
        //lcd.setCursor(0,0);
        //Serial.println("S:1 Ramp to Soak");
        /* Display the current time from start of the reflow process in Minutes:seconds */
        printTime();

        /* State Changed Beep */
        tone(BUZZER_PIN,2048,1000);
        buzzer_ms = 1250;

        START = 0;
        PWM = 100; // PWM of 100%
      }
      break;
    case 1 : // Ramp to soak state
      // 100% power

      /* Abort if temperature is not above 50C in 1:20 */
      if (Temp <= AB_TEMP && minutes >= 1 && seconds >= 20) {
        /* Abort Message */
        lcd.setCursor(0, 0);
        //Serial.println("System Aborted  ");
        lcd.setCursor(0,1);
        //Serial.println("Check TC        ");
        
        /* Beep */
        tone(BUZZER_PIN,2048,3000);
        buzzer_ms = 3000;
        delay(2000); // wait 2 seconds

        /* Set variables to default */
        PWM = 0;
        state = 0;
        sec = 0;
        START = 0;
        lcd.setCursor(0,0);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("                ");
      }
      /* Otherwise advance if greater than soak temperature */
      else if (Temp >= TEMP_SOAK) {
        /* Display State Message */
      //  lcd.setCursor(0,0);
        //Serial.println("S:2 Soak            ");

        PWM = 0;
        
        printTime();
        buzzer_ms = 1250;
        tone(BUZZER_PIN,2048,1000);
        
        state = 2; // Next state
        sec = 0;
      } 
      /* The next 5 else if statements:
         changing the PWM based on the temperature closer to the setpoint  */
      else if (Temp >= TEMP_SOAK - 5) {
        PWM = 0;
      }
      else if (Temp >= TEMP_SOAK - 25) {
        PWM = 10;
      }
      else if (Temp >= TEMP_SOAK - 30) {
        PWM = 20;
      }
      else if (Temp >= TEMP_SOAK - 35) {
        PWM = 30;
      }
      else if (Temp >= TEMP_SOAK - 40) {
        PWM = 40;
      }
      break;
    case 2 : // Preheat / Soak state
      /* Advance if time has elapsed */
      if ( sec >= TIME_SOAK) {
        /* Display State Message */
        lcd.setCursor(0,0);
        //Serial.println("S:3 Ramp to Peak    ");

        PWM = 100;

        printTime();
        buzzer_ms = 1250;
        tone(BUZZER_PIN,2048,1000);
        
        state = 3; // advance to next state
      }
      /* Start the oven slightly early */
      else if ( sec >= 30) {
        PWM = 50;
      }
      break;
    case 3 : // Ramp to peak state
      /* Advance if past threshold */
      if (Temp >= TEMP_REFL) {
        /* Display State Message */
        lcd.setCursor(0,0);
        //Serial.println("S:4 Reflow          ");

        PWM = 10; // 30% PWM
        printTime();
        buzzer_ms = 1250;
        tone(BUZZER_PIN,2048,1000);
        
        state = 4; // Advance to next state
        sec = 0;
      }
      /* Manual changes to PWM due to temperature */
      else if (Temp >= TEMP_REFL - 10) {
        PWM = 40;
      }
      break;
    case 4 : // This is the reflow state
      /* Advance to next state if finished reflowing */
      if (sec >= TIME_REFL) {
        /* Display State Message */
        lcd.setCursor(0,0);
        //Serial.println("S:5 Cooling         ");
        
        PWM = 0;
        printTime();
        buzzer_ms = 5000;
        tone(BUZZER_PIN,2048,5000);
        
        state = 5; // Next state
      }
      /* Temperature above abort threshold, abort */
      else if ( Temp >= TEMP_ABRT) {
        PWM = 0;
        /* Display Abort message */
        lcd.setCursor(0, 0);
        //Serial.println("Abort           ");
        lcd.setCursor(0,1);
        //Serial.println("Temp too hot    ");
        buzzer_ms = 3000;
        tone(BUZZER_PIN,2048,3000);
        delay(2000); // Delay for message before clear

        lcd.setCursor(0,0);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("                ");
        state = 5; // Jump to cooling right away after temperature got too hot
      }
      break;
    case 5 : // Cooling state
      // 0% power
      PWM = 0;
      /* When temperature is cool enough to touch PCB let the user know*/
      if (Temp <= TEMP_COOL) {
        for (int i = 0; i < 6; i++) {
          coolingLoop(); // Beep 6 times
        }
        /* Clear screen */
        lcd.setCursor(3,0);
        lcd.print("                    ");
        lcd.setCursor(7,1);
        lcd.print("                    ");
        
        /* Reset variables */
        state = 0;
        START =0;
      }
      break;
  }
}
