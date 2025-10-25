// Pin config
#define PIN_BTN_1 8
#define PIN_BTN_2 9

// Loop config
#define L_STATE_INIT 0
#define L_STATE_CONFIG 1
#define L_STATE_RUNNING 2

// loop state
int l_state;

// pin states
int n_pins = 2;
int pin_ids[] = {PIN_BTN_1, PIN_BTN_2};
int pin_states[] = {0, 0};

void setup(){
  Serial.begin(9600);

  // Setup pins
  for (int i=0; i<n_pins;i++){
    pinMode(pin_ids[i], INPUT);
    digitalWrite(pin_ids[i], HIGH);
  }
  
  // Setup loop state
  l_state = L_STATE_INIT;
}

void loop(){
  switch(l_state){
    case L_STATE_INIT:
      l_init();
      break;
    case L_STATE_CONFIG:
      l_config();
      break;
    case L_STATE_RUNNING:
      l_run();
      break;
  }
}

void l_init(){
  Serial.print("0");
  while (Serial.available() == 0) {}     //wait for data available
  int buf = Serial.read();

  // Set loop running
  l_state = L_STATE_RUNNING;
}

void l_config(){
  l_state = L_STATE_RUNNING;
}

void l_run(){
  // NOTE: this is a blocking operation, this makes an assumption that
  // one key will be pressed at the time. In future, can change this to
  // press and release signals.

  for (int i=0; i<n_pins;i++){
    // wait for the key to be pressed and released before moving ahead
    while (digitalRead(pin_ids[i])){
      pin_states[i]=1;
    }

    // if state is high send signal
    if (pin_states[i]){
      Serial.print(i+1);
    }

    // reset the state
    pin_states[i] = 0;
  }

  // signal delay
  delay(100);
}
