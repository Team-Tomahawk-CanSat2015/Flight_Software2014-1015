/*********************************************************************************************************/
/* Stage transition and task Alogorithms/
**********Sensor data reference************
[0] - temp (celcius-1)
[1] - latitude
[2] - longitude
[3] - altitude (m-0.1)
[4] - descent rate (m/s - 0.1)
[5] - voltage (volts-0.05)
[6] - x axis angle, "alpha" (degrees)  //Look at IMU for axis referencing
[7] - y axis angle, "alpha" (degrees)  //Look at IMU for axis referencing
[8] - z axis angle, "alpha" (degrees)  //Look at IMU for axis referencing
[9]- z_axis roll Rate (deg/s)
/*****************Relevant Times reference ***************
RocketBurn_time = 2s;
Delay_time = 9 sec;
SatDeployDelay = 5 sec;
Nichromeburn_time = 4 sec:
/*********************************************************************************************************/

//Non time (reset protect) important
unsigned long prevToneTime=0;
boolean landedSetupFlag = false;
boolean descentSetupFlag = false;

//initializaiton task
void initialize(){
  
  if (initialize_time==0)
  {
    initialize_time = a_time-(millis()-preResetTime+1)/1000;
//    Serial.print("**Initialize_time: ");
//    Serial.print(initialize_time);
//    Serial.print("  **a_time: ");
//    Serial.println(a_time);
  }
  if (fix_time==0)
    fix_time = a_time;
  else
  {
    if (a_time-fix_time>altCalibrationDuration && sensor_data[4] <0.1 && sensor_data[4]>-0.1)
    {
       ground_alt = sensor_data[3];
       state = 1;
    }
  }
}


void launch_wait() {
  
  /********FUNCTION task*********/
   //Reset stuff here
  
  /********Transition Check*********/
  if (sensor_data[3] >  acsentTransitionAlt+ground_alt && sensor_data[4] <-5) { 
       state = 2;
       stateStartTime = a_time; //Register time of liftoff
  }
}

void ascent() {
    /********FUNCTION task*********/
   //NO function task for ascent

   /********Transition Check*********/
  if ( (a_time - stateStartTime) >  ( RocketBurn_time + RocketDelay_time )  ) { //I guess this is where we need a RTC however i used packet count for now.
                                                                               //our GPS had an RTC ANS I WILL probably activate that
       state = 3;
       stateStartTime = a_time;
  }
}

  void rocketDeployment_Stabilization() { //CANSAT stabilization/deployment
    /********FUNCTION task*********/
   //NO function task for ascent

   /********Transition Check*********/
  if ( (a_time - stateStartTime) >  (PayloadDeployDelay_time) && sensor_data[4]> 5) {  // if (9 +2 + 2) seconds has passed (9 sec delay + 2 sec burn + 5 sec to stabilize) 
       state = 4;
       stateStartTime = a_time;
  }
}

  void seperation() {
      /********FUNCTION task*********/
   digitalWrite(nichromePin, HIGH);  //Nichrome BURN BBAABYY!!!!!!!!!!!!!

   /********Transition Check*********/
  if ( (a_time - stateStartTime) >  ( WireBurn_time)) {
       state = 5;
       init_Heading = sensor_data[6]; //initialize heading for fin stabilization
       digitalWrite(nichromePin, LOW);
  }
}

  void descent() {
    
    if (!descentSetupFlag)
      {
        //Configure servo pins
        servo1.attach (servoOnePin);
        servo2.attach (servoTwoPin);
        descentSetupFlag = true;
      }
   /********FUNCTION task*********/
   stabilize(init_Heading, sensor_data[4]);  //Fins Activate !!!!!!!!!!!!!

   /********Transition Check*********/
  if (sensor_data[3] < landedTransitionAlt +ground_alt) {
      servo1.detach();
      servo2.detach();
      state = 6;
  }
  
  
  }
  void landed() {
      if (!landedSetupFlag)
      {
        setupGPS();
        landedSetupFlag = true;
      }
      getGPSdata(&sensor_data[1],&sensor_data[2]);
      if (millis()-prevToneTime>=10000)
      {
        tone (buzzerPin, 262,4000);
        prevToneTime = millis();
      }
  }
