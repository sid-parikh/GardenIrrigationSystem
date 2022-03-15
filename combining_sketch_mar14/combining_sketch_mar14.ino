#include <cstdint>

constexpr byte moisture_sensor_pin{0};
constexpr byte moisture_power_pin{0};

constexpr byte rain_valve_pin{0};
constexpr byte tap_valve_pin{0};

constexpr byte flow_sensor_pin{0};
constexpr byte flow_power_pin{0};

volatile int flow_frequency{0};
double total_flow{0};
unsigned long cloop_time{0};

enum class State {
  watering,
  sensing,
  sleeping
};


enum class WateringState {
  rain,
  tap,
  none
};

// Track the current status
State system_status;

// Track the watering status
WateringState watering_status{WateringState::none};

// Called every loop when the state is "watering"
State water() {
  switch(watering_status) {
    case WateringState::none: 
      // Watering just started
      // Open rainwater valve
      digitalWrite(rain_valve_pin, HIGH);
      // Monitor flow meter pulses
      attachInterrupt(digitalPinToInterrupt(flow_sensor_pin), flow, RISING);
      cloop_time = millis();
      watering_status = WateringState::rain;
      
      return State::watering;
      break;
    case WateringState::rain:
      // Rainwater valve is open
      // Only act every second
      unsigned long current_time = millis();
      if (currentTime >= cloop_time + 1000) {
        // It's been a second, act on the flow meter and then mark the new time
        cloop_time = currentTime;

        // Save current register state (includes interrupts flag)
        uint8_t oldSREG = SREG;
        // disable interrupts while we read flow_frequency or we might get an
        // inconsistent value (e.g. in the middle of a write to flow_frequency)
        noInterrupts();
        int latestFlow = flow_frequency;
        flow_frequency = 0;
        // return register to old state
        SREG = oldSREG;

        // (Pulse frequency in Hz) / (7.5) = flow rate in L/minute
        double one_minute = latestFlow / 7.5;
        
        
      }
  }

  return State::sleeping;
}

// Called every loop when the state is "sensing"
State sense() {
  return State::watering;
}

// Called every loop when the state is "sleeping"
State sleep() {
  return State::sleeping;  
}

// This function is the interrupt called on rising edges
void flow() {
  flow_frequency++;
}

// Setup the pins on startup
void initGPIO() {
  pinMode(moisture_sensor_pin, INPUT);
  pinMode(moisture_power_pin, OUTPUT);
  
  pinMode(rain_valve_pin, OUTPUT);
  pinMode(tap_valve_pin, OUTPUT);

  pinMode(flow_sensor_pin, INPUT);
  pinMode(flow_power_pin, OUTPUT); // NEEDS TESTING 
  // Pull up flow pin
  digitalWrite(flow_sensor_pin, HIGH);
  
}

// On Power Cycle
void setup() {
  initGPIO();
  
  /* Check for BT connection and write preferences as needed here ..... */

  // Make no assumptions and go back into sensing mode.
  system_status = State::sensing;
}

void loop() {
  switch (system_status) {
    case State::sensing: system_status = sense(); break;
    case State::watering: system_status = water(); break;
    case State::sleeping: system_status = sleep(); break;
  }
}
