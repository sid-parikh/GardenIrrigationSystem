#include <EEPROM.h>

constexpr byte moisture_sensor_pin{0};
constexpr byte moisture_power_pin{0};

constexpr byte rain_valve_pin{0};
constexpr byte tap_valve_pin{0};

constexpr byte flow_sensor_pin{0};
constexpr byte flow_power_pin{0};

constexpr int eeprom_address_moisture_level{0};
constexpr int eeprom_size{1};

constexpr double moisture_diff_to_flow_ratio{0.5};

constexpr int microseconds_to_seconds{1000000};

// Should probably be 0 or a very small number
constexpr double minimum_water_flow_rate{0};

volatile int flow_frequency{0};
double total_flow{0};
double target_flow{0};
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

void start_flow_meter() {
  digitalWrite(flow_power_pin, HIGH);

  // Monitor flow meter pulses
  attachInterrupt(digitalPinToInterrupt(flow_sensor_pin), flow, RISING);
  cloop_time = millis();
}

bool check_target_met_and_close(byte valve_pin) {
  if (total_flow >= target_flow) {
    // Reached the target
    // Close rainwater valve
    digitalWrite(rain_valve_pin, LOW);
    // Stop monitoring flow meter
    detachInterrupt(digitalPinToInterrupt(flow_sensor_pin));
    flow_frequency = 0;
    total_flow = 0;
    
    return true;
  }
  return false;
}

// Returns the current flow rate in Liters per minute
// Assumes
double get_flow_rate(unsigned long current_time) {
  // Save current register state (includes interrupts flag)
  uint8_t oldSREG = SREG;
  // disable interrupts while we read flow_frequency or we might get an
  // inconsistent value (e.g. in the middle of a write to flow_frequency)
  noInterrupts();
  double latestFlow {flow_frequency};
  flow_frequency = 0;
  // return register to old state
  SREG = oldSREG;

  // Latest flow is in Hz
  latestFlow /= ((current_time - cloop_time) / 1000.0);

  // (Pulse frequency in Hz) / (7.5) = flow rate in L/minute
  return latestFlow / 7.5;
}

// Called every loop when the state is "watering"
State water() {
  switch(watering_status) {
    case WateringState::none: 
      // Watering just started
      // Open rainwater valve
      digitalWrite(rain_valve_pin, HIGH);
      
      start_flow_meter();

      watering_status = WateringState::rain;
      
      return State::watering;
      break;
    case WateringState::rain:
      // Rainwater valve is open
      // Only act every second
      unsigned long current_time = millis();
      if (currentTime >= cloop_time + 1000) {
        // It's been a second, act on the flow meter and then mark the new time
       
        // Convert L/min to L/sec and then add to total flow by mutiplying flow rate in L/sec by the time in seconds
        total_flow += (get_flow_rate(current_time) / 60.0) * ((current_time - cloop_time) / 1000.0));

        if (check_target_met_and_close(rain_valve_pin)) {
          // Target has been met; enter sleep state
          watering_status = WateringState::none;
          return State::sleeping;
        }

        // If rainwater isn't working, switch to tap water
        if (!(one_minute > minimum_water_flow_rate)) {
          // Close rainwater valve
          digitalWrite(rain_valve_pin, LOW);
          // Open tap water valve
          digitalWrite(tap_valve_pin, HIGH);
          watering_status = WateringState::tap;
        }

        // Mark new time
        cloop_time = currentTime;

        return State::watering;
      }
      break;
    case WateringState::tap:
      // Tap water valve is open
      // Only act every second
      unsigned long current_time = millis();
      if (currentTime >= cloop_time + 1000) {
        // Convert L/min to L/sec and then add to total flow by mutiplying flow rate in L/sec by the time in seconds
        total_flow += (get_flow_rate(current_time) / 60.0) * ((current_time - cloop_time) / 1000.0));

        if (check_target_met_and_close(rain_valve_pin)) {
          // Target has been met; enter sleep state
          watering_status = WateringState::none;
          return State::sleeping;
        }

        if (check_target_met_and_close(tap_valve_pin)) {
          // Target has been met; enter sleep state
          watering_status = WateringState::none;
          return State::sleeping;
        }

        // Mark new time
        cloop_time = currentTime;

        return State::watering;
      }
      break;
  }

  return State::sleeping;
}

// Called every loop when the state is "sensing"
// Designed to only last one loop long
State sense() {
  // Get a measurement from the moisture sensor
  digitalWrite(moisture_power_pin, HIGH);
  delay(10);
  int moisture_level = analogRead(moisture_sensor_pin);
  digitalWrite(moisture_power_pin, LOW);

  // Read ESP32 Flash memory for minimum moisture level
  byte minimum_moisture_level = EEPROM.read(eeprom_address_moisture_level);

  // If the moisture level is below the minimum, switch to watering mode
  if (moisture_level < minimum_moisture_level) {

    /* TODO: add weather checking */

    // Calculate target watering amount
    target_flow = (minimum_moisture_level - moisture_level) * moisture_diff_to_flow_ratio;

    // Enter watering mode
    return State::watering;
  } else {
    // Check again later
    return State::sleeping;
  }
}

// Called every loop when the state is "sleeping"
// Should only run once because, well, the board will sleep
State sleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  esp_deep_sleep_start();

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
  
  // Initialize EEPROM
  EEPROM.begin(eeprom_size);

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
