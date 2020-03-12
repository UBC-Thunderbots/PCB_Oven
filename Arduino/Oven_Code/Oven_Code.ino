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
  //attachInterrupt(digitalPinToInterrupt(START_KEY), START_BUTTON, FALLING);
  attachInterrupt(digitalPinToInterrupt(ABORT_KEY), ABORT_BUTTON,FALLING);
}

/* Interrupt Service Routine for Timer 1 */
ISR(TIMER1_OVF_vect)        // interrupt service routine for complement oven
{
  TCNT1 = timer1_counter;   // preload timer

  pulse_on = 1000 - PWM*10;
  if ( Count1ms >= 1000){
    /* send temperature to serial port and LCD every second */
    Temp = readTemp();
    printTemp();
    Serial.println(Temp,1);
    
    state_seconds += 1;
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

  digitalWrite(PULSE_PIN,PULSE);

  //increment the ms counter
  Count1ms += 1;
}

void loop() { 
  //Temp = readTemp();

  

  /* Display State on display as long as there is no abort */
  if (ABORT == 0) {
    lcd.setCursor(0, 0);
    lcd.print("S:");
    lcd.print(state);
    if (state == 0) {
      lcd.print("               ");
    }
  }
  else {
    // abort command
    ABORT_MANUAL();
    // reset abort variable
    ABORT = 0;
    skip_python = 1;
  }
  
  /* Print Controller runtime since start */
  if (state != 0) {
    printTime();
  }
 
  // FSM
  switch (state) {
    case 0 : 
      PWM = 0;
      // Obtain thermal profile parameters
      if (skip_python == 0) {
        default_param = receiveParameter();
        if (default_param == 2)
          default_param = 0;
        custom.soakTemp = default_param ? TEMP_SOAK : receiveParameter();
        custom.soakTime = default_param ? TIME_SOAK : receiveParameter();
        custom.reflowTemp = default_param ? TEMP_REFL : receiveParameter();
        custom.reflowTime = default_param ? TIME_REFL : receiveParameter();
      }
      else {
        lcd.setCursor(0, 0);
        lcd.print("S:0                    ");
      }


      
      while(digitalRead(START_KEY) != 0);
      while(digitalRead(START_KEY) == 0);                                  // Wait for 'set' button to be released      
      delay(50);
      // wait until start key is pressed to start the reflow cycle 
      //if (START == 1) {
        
      if (default_param == 0 ) {
        Serial.print("Start\n");
      }
      initialiseTimer1();
      /* Variables set to default */
      state_seconds = 0;
      seconds = 0;
      minutes = 0;
      state = 1;
      
      /* Display next State message */
      lcd.setCursor(3,0);
      lcd.print(" Ramp to Soak");
      
      /* Display the current time from start of the reflow process in Minutes:seconds */
      printTime();

      /* State Changed Beep */
      tone(BUZZER_PIN,1500,1000);
      
      //START = 0;
      PWM = 100; // PWM of 100%
      //}
      break;
    case 1 : // Ramp to soak state
      // 100% power

      /* Abort if temperature is not above 50C in 1:20 */
      if (Temp <= AB_TEMP && minutes >= 1 && seconds >= 20) {
        /* Abort Message */
        lcd.setCursor(0, 0);
        lcd.print("System Aborted  ");
        lcd.setCursor(0,1);
        lcd.print("Check TC        ");
        
        /* Beep */
        tone(BUZZER_PIN,2048,3000);
        delay(2000); // wait 2 seconds

        /* Set variables to default */
        PWM = 0;
        state = 0;
        state_seconds = 0;
        START = 0;
        lcd.setCursor(0,0);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("                ");
      }
      /* Otherwise advance if greater than soak temperature */
      else if (Temp >= custom.soakTemp) {
        /* Display State Message */
        lcd.setCursor(3,0);
        lcd.print(" Soak               ");
        
        PWM = 0;
        
        printTime();
        tone(BUZZER_PIN,2048,1000);
        
        state = 2; // Next state
        state_seconds = 0;
      } 
      /* The next 5 else if statements:
         changing the PWM based on the temperature closer to the setpoint  */
      else if (Temp >= custom.soakTemp - 15) {
        PWM = 10;
      }
      else if (Temp >= custom.soakTemp - 20) {
        PWM = 20;
      }
      else if (Temp >= custom.soakTemp - 30) {
        PWM = 30;
      }

      break;
    case 2 : // Preheat / Soak state
      /* Advance if time has elapsed */
      if ( state_seconds >= custom.soakTime) {
        /* Display State Message */
        lcd.setCursor(3,0);
        lcd.print(" Ramp to Peak    ");

        PWM = 100;

        printTime();
        tone(BUZZER_PIN,2048,1000);
        
        state = 3; // advance to next state
      }
      /* Start the oven slightly early */
      else if ( state_seconds >= 30) {
        PWM = 50;
      }
      break;
    case 3 : // Ramp to peak state
      /* Advance if past threshold */
      if (Temp >= custom.reflowTemp) {
        /* Display State Message */
        lcd.setCursor(3,0);
        lcd.print(" Reflow          ");
        
        PWM = 5; // 5% PWM
        printTime();
        tone(BUZZER_PIN,2048,1000);
        
        state = 4; // Advance to next state
        state_seconds = 0;
      }
      /* Manual changes to PWM due to temperature */
      else if (Temp >= custom.reflowTemp - 10) {
        PWM = 30;
      }
      break;
    case 4 : // This is the reflow state
      /* Advance to next state if finished reflowing */
      if (state_seconds >= custom.reflowTime) {
        /* Display State Message */
        lcd.setCursor(3,0);
        lcd.print(" Cooling         ");
        
        PWM = 0;
        printTime();
        tone(BUZZER_PIN,2048,5000);
        
        state = 5; // Next state
      }
      /* Temperature above abort threshold, abort */
      else if ( Temp >= TEMP_ABRT) {
        PWM = 0;
        
        /* Display Abort message */
        lcd.setCursor(0, 0);
        lcd.print("Abort           ");
        lcd.setCursor(0,1);
        lcd.print("Temp too hot    ");
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
        START = 0;
      }
      break;
  }
}
