# L552_AMS Project Code Analysis

## Project Overview

**L552_AMS** is an **STM32L552 Microcontroller-based Battery Management System (BMS)** designed to manage and monitor **5 battery segments** using the **ADBMS6830 Battery Monitoring IC** from Analog Devices.

### Key Characteristics:
- **Target MCU**: STM32L552ZETXQ (ARM Cortex-M33, 160KB SRAM, 512KB Flash)
- **Application**: Battery Management System with advanced monitoring and diagnostics
- **Communication Protocols**: SPI, I2C, CAN-FD (FDCAN), UART
- **Monitored Segments**: 5 independent battery segments (SEG1-SEG5)
- **Cells per Segment**: 12 cells each (total 60 cells monitored)

---

## Architecture & System Design

### System Block Diagram

```
STM32L552 MCU (Main Controller)
├─ SPI1 ──→ ADBMS6830 ICs (Cell Voltage Monitoring)
├─ I2C1 ──→ Auxiliary Devices
├─ ADC1 ──→ Temperature Sensors
├─ USART3 ──→ Debug Serial Output
├─ LPUART1 ──→ Additional Serial Communication
├─ FDCAN1 ──→ CAN Network (Data Transmission)
├─ TIM2, TIM3, TIM5 ──→ Timing & Interrupt Management
└─ GPIO ──→ LED Control (GREEN, RED, BLUE)
```

---

## Core Module Descriptions

### 1. **Main Application Module** ([main.c](Core/Src/main.c))

**Purpose**: Entry point and main control loop for the BMS

**Key Global Variables**:
```c
SEG_PARAMS SEG1, SEG2, SEG3, SEG4, SEG5;  // Voltage & temperature data
SEG_BSTAT SEG1_B, SEG2_B, SEG3_B, SEG4_B, SEG5_B;  // Battery status flags
uint16_t TS_Current;     // Total system current
uint8_t AMS_SOC;         // State of Charge
uint8_t AMS_SAFE;        // Safety status flag
uint16_t AMS_Charg_I;    // Charging current
uint16_t AMS_Charg_V;    // Charging voltage
```

**Main Loop Flow**:
1. Initialize peripherals (GPIO, ADC, I2C, SPI, UART, CAN, Timers)
2. Initialize CAN data structures
3. Initialize ADBMS monitoring loop
4. Start TIM3 interrupt
5. **Infinite Loop**:
   - Call `adbms_readCell_loop()` - Read cell voltages
   - Call `adbmsReinitMain()` - Re-initialize if needed
   - Delay 3ms

### 2. **Data Structures** ([main.h](Core/Inc/main.h))

#### SEG_PARAMS Structure
```c
typedef struct {
    float IC_T;           // IC temperature (°C)
    float CELL_V[12];     // Cell voltages (V) - 12 cells
    float CELL_T[12];     // Cell temperatures (°C) - 12 cells
} SEG_PARAMS;
```
- **Usage**: Stores processed voltage and temperature data for each segment
- **5 Instances**: SEG1, SEG2, SEG3, SEG4, SEG5

#### SEG_BSTAT Structure
```c
typedef struct {
    uint8_t M1to8;        // Status flags for cells 1-8
    uint8_t M9to12;       // Status flags for cells 9-12
} SEG_BSTAT;
```
- **Usage**: Tracks individual cell fault flags (over-voltage, under-voltage, etc.)

---

### 3. **ADBMS6830 Module** ([Core/ADBMS6830/](Core/ADBMS6830/))

**Purpose**: Interface driver for Analog Devices BMS IC handling cell voltage monitoring

#### Key Components:

##### **adbms_main.h** ([Core/ADBMS6830/inc/adbms_main.h](Core/ADBMS6830/inc/adbms_main.h))
- `adbms_main()` - Main BMS routine
- `adbms_init_loop()` - Initialize BMS IC
- `adbms_readCell_loop()` - Read cell voltages continuously
- `adbms_startCellConv_loop()` - Start ADC conversion
- `adbms_readTempToggle()` - Read temperature data
- `testOpenWire()` - Diagnostic open-wire test
- `adbmsReinit()` - Re-initialize specific IC

##### **adBms_Application.h** ([Core/ADBMS6830/inc/adBms_Application.h](Core/ADBMS6830/inc/adBms_Application.h))

**Measurement Functions**:
- `adBms6830_read_cell_voltages()` - Read cell voltages
- `adBms6830_read_aux_voltages()` - Read auxiliary voltages
- `adBms6830_read_status_registers()` - Read status/diagnostic data
- `adBms6830_read_s_voltages()` - Read S-voltage (thermistor, power supply reference)
- `adBms6830_read_avgcell_voltages()` - Read averaged cell voltages
- `adBms6830_read_fcell_voltages()` - Read filtered cell voltages

**Configuration Functions**:
- `adBms6830_init_config()` - Initialize configuration registers
- `adBms6830_write_config()` - Write config parameters
- `adBms6830_read_config()` - Read config parameters

**Diagnostic Functions**:
- `adBms6830_run_osc_mismatch_self_test()` - Oscillator test
- `adBms6830_run_thermal_shutdown_self_test()` - Thermal test
- `adBms6830_cell_openwire_test()` - Open-wire detection
- `adBms6830_redundant_cell_openwire_test()` - Redundant open-wire test

**Control Functions**:
- `adBms6830_set_dcc_discharge()` - Enable cell discharge
- `adBms6830_clear_dcc_discharge()` - Disable cell discharge
- `adBms6830_soft_reset()` - Reset BMS IC
- `adBms6830_enable_mute()` - Mute fault detection
- `adBms6830_disable_mute()` - Enable fault detection

#### **adBms6830Data.h** - Data Definitions

**ADBMS6830 IC Specifications**:
```c
#define CELL    12      // 12 cell voltage channels
#define AUX     12      // 12 auxiliary channels (thermistors, references)
#define RAUX    10      // 10 redundant auxiliary channels
#define PWMA    12      // 12 PWM A channels (discharge control)
#define PWMB    4       // 4 PWM B channels
#define COMM    3       // 3 GPIO communication registers
#define RSID    6       // Serial ID size (bytes)
```

**Configuration Register Structures**:

```c
typedef struct {
    uint8_t refon;      // Voltage reference enable
    uint8_t cth;        // Temperature sensor configuration
    uint8_t flag_d;     // Fault flag delay
    uint8_t soakon;     // Second order active filter enable
    uint8_t owrng;      // Open-wire detection range
    uint8_t owa;        // Open-wire detection algorithm
    uint16_t gpo;       // GPIO output configuration (10-bit)
    uint8_t snap;       // Snapshot enable
    uint8_t mute_st;    // Mute status
    uint8_t comm_bk;    // Communication back-up
    uint8_t fc;         // Fault clear configuration
} cfa_;

typedef struct {
    uint16_t vuv;       // Under-voltage threshold (ADC codes)
    uint16_t vov;       // Over-voltage threshold (ADC codes)
    uint8_t dtmen;      // Discharge timer enable
    uint8_t dtrng;      // Discharge timer range
    uint8_t dcto;       // Discharge time-out (6-bit)
    uint16_t dcc;       // Discharge current configuration
} cfb_;
```

**Data Structures for Measurements**:
```c
typedef struct { int16_t c_codes[CELL]; } cv_;      // Cell Voltages
typedef struct { int16_t ac_codes[CELL]; } acv_;    // Averaged Cells
typedef struct { int16_t fc_codes[CELL]; } fcv_;    // Filtered Cells
typedef struct { int16_t sc_codes[CELL]; } scv_;    // S-Voltages
typedef struct { int16_t a_codes[AUX]; } ax_;       // Aux Voltages
typedef struct { int16_t ra_codes[RAUX]; } rax_;    // Redundant Aux
```

#### **adBms6830GenericType.h** - Type Definitions

**Key Enumerations**:
```c
RD      - Redundant measurement (RD_ON/RD_OFF)
CH      - Auxiliary channel select
CONT    - Continuous vs Single measurement
OW_C_S  - Open-wire detection (OW_OFF_ALL_CH, OW_ON_*)
OW_AUX  - Auxiliary open-wire detection
PUP     - Pull-up/pull-down current selection
DCP     - Discharge permit
RSTF    - Reset filter
ERR     - Error injection for testing
LOOP_MEASURMENT - Enable/disable measurements
TYPE    - Measurement type (CELL, REDUNDANT, etc.)
OW_C_S  - Open-wire config
PUP     - Pull-up config
```

#### **adBms6830ParseCreate.h** - Command Building

- `build_cmd()` - Construct SPI commands
- `parse_result()` - Interpret IC responses
- `check_pec()` - Packet Error Code validation

#### **mcuWrapper.h** - MCU Abstraction Layer

- Hardware abstraction for SPI communication
- GPIO control for IC reset/enable
- Timer/delay functions
- UART for debugging

#### **serialPrintResult.h** - Debug Output

- Print voltage/temperature data
- Display status registers
- Debug fault information

---

### 4. **Application Configuration** ([adBms_Application.c](Core/ADBMS6830/src/adBms_Application.c))

**Configuration Parameters**:
```c
#define TOTAL_IC 1                              // Number of ADBMS ICs
cell_asic IC[TOTAL_IC];                         // IC instance array

// ADC Measurement Settings
RD REDUNDANT_MEASUREMENT = RD_OFF;              // Disable redundant
CH AUX_CH_TO_CONVERT = GPIO10;                  // Aux channel to read
CONT CONTINUOUS_MEASUREMENT = SINGLE;           // Single measurements
OW_C_S CELL_OPEN_WIRE_DETECTION = OW_OFF_ALL_CH; // Open-wire detection
OW_AUX AUX_OPEN_WIRE_DETECTION = AUX_OW_OFF;    // Aux open-wire
PUP OPEN_WIRE_CURRENT_SOURCE = PUP_UP;          // Pull-up current
DCP DISCHARGE_PERMITTED = DCP_OFF;              // Disable discharge
RSTF RESET_FILTER = RSTF_OFF;                   // Keep filter
ERR INJECT_ERR_SPI_READ = WITHOUT_ERR;          // No error injection

// Voltage Thresholds
const float OV_THRESHOLD = 4.2;                 // Over-voltage: 4.2V
const float UV_THRESHOLD = 2.6;                 // Under-voltage: 2.6V

// Fault Detection Thresholds
const int OWC_Threshold = 2000;                 // Open-wire cell: 2000mV
const int OWA_Threshold = 50000;                // Open-wire aux: 50000mV

// Measurement Timing
const uint32_t LOOP_MEASUREMENT_COUNT = 1;      // Iterations per loop
const uint16_t MEASUREMENT_LOOP_TIME = 10;      // Loop period: 10ms

// Enabled Measurements (ENABLED/DISABLED)
LOOP_MEASURMENT MEASURE_CELL = ENABLED;         // Cell voltages
LOOP_MEASURMENT MEASURE_AVG_CELL = ENABLED;     // Averaged cells
LOOP_MEASURMENT MEASURE_F_CELL = ENABLED;       // Filtered cells
LOOP_MEASURMENT MEASURE_S_VOLTAGE = ENABLED;    // Supply voltages
LOOP_MEASURMENT MEASURE_AUX = DISABLED;         // Auxiliary
LOOP_MEASURMENT MEASURE_RAUX = DISABLED;        // Redundant aux
LOOP_MEASURMENT MEASURE_STAT = DISABLED;        // Status registers
```

---

### 5. **Hardware Peripherals** ([Core/Inc/](Core/Inc/))

#### **GPIO Configuration** ([gpio.h](Core/Inc/gpio.h))

**LED Indicators**:
- `LED_GREEN_Pin` (GPIOC-7)
- `LED_RED_Pin` (GPIOA-9)
- `LED_BLUE_Pin` (GPIOB-7)

**Data Pins** (D2-D9 for external signaling):
- D2, D3, D4, D5, D6, D7, D8, D9

**Control Pins**:
- `SPI_CS_Pin` (GPIOD-14) - SPI chip select for ADBMS IC
- `USER_BUTTON_Pin` (GPIOC-13)

#### **SPI Communication** ([spi.h](Core/Inc/spi.h))

- **Interface**: SPI1
- **Purpose**: Communication with ADBMS6830 IC
- **Speed**: Configured via STM32CubeMX
- **Handle**: `hspi1`

#### **CAN Network** ([fdcan.h](Core/Inc/fdcan.h))

- **Interface**: FDCAN1 (Flexible Data-rate CAN)
- **Purpose**: Transmit BMS data to vehicle network

**CAN Message Structures**:
```c
typedef struct {
    uint8_t datas[25];              // Payload (25 bytes)
    FDCAN_TxHeaderTypeDef header;   // CAN header
} CanTxMsg;

// Predefined messages:
CanTxMsg canSeg1, canSeg2, canSeg3, canSeg4, canSeg5;  // Segment data
CanTxMsg canAccuStat;       // Accumulator status
CanTxMsg canStat;           // General status
CanTxMsg canChargeStat;     // Charging status
```

**CAN Functions**:
- `canFraming()` - Format BMS data into CAN frames
- `CAN_Data_Init()` - Initialize CAN message structures
- `CAN_DataTX_1s()` - Transmit data (1-second interval)

#### **ADC** ([adc.h](Core/Inc/adc.h))

- **Interface**: ADC1
- **Purpose**: Acquire analog signals (temperature sensors, current measurement)

#### **I2C** ([i2c.h](Core/Inc/i2c.h))

- **Interface**: I2C1
- **Purpose**: Communicate with auxiliary I2C devices

#### **UART/USART** ([usart.h](Core/Inc/usart.h))

- **LPUART1** & **USART3**: Serial communication for debugging
- **Baud Rate**: Standard (typically 115200)
- **Purpose**: Debug output, monitoring

#### **Timers** ([tim.h](Core/Inc/tim.h))

- **TIM2, TIM3, TIM5**: Provide timing for:
  - Periodic measurement intervals
  - CAN message transmission
  - Interrupt generation for control timing

---

## Data Flow & Measurement Loop

### Typical Measurement Sequence:

```
Main Loop (every 3ms)
├─ adbms_readCell_loop()
│  ├─ Start cell voltage ADC conversion
│  ├─ Read previous conversion results
│  ├─ Parse voltage data into SEG1-5.CELL_V[]
│  ├─ Convert ADC codes to voltages (mV)
│  ├─ Check OV/UV thresholds
│  ├─ Update SEG1_B-5_B fault flags
│  └─ Return to main loop
│
├─ adbmsReinitMain()
│  ├─ Monitor IC communication status
│  ├─ Re-initialize if faults detected
│  └─ Reset error flags if needed
│
└─ HAL_Delay(3ms)

CAN Transmission (1s interval)
├─ canFraming()
│  ├─ Pack SEG1-5 voltage data into CAN frames
│  ├─ Include temperature & status
│  └─ Update CAN message buffers
│
└─ CAN_DataTX_1s()
   ├─ Transmit canSeg1-5 (segment data)
   ├─ Transmit canStat (status)
   ├─ Transmit canChargeStat (charging info)
   └─ Transmit canAccuStat (accumulator status)
```

---

## Voltage Conversion

### ADC to Voltage Formula:
```
Cell Voltage = (ADC_Code × 100µV) to (ADC_Code × 300µV)
```
Typical: 10-bit ADC with ~0.1V LSB per code

### Temperature Conversion:
```c
float get_temp_from_voltage(float voltage)  // Input: voltage from thermistor
float v2Temp(float x)                       // Voltage to temperature
float get_temp(float y)                     // Alternative temperature calc
```

---

## Fault Detection & Protection

### Thresholds:
- **Over-Voltage**: 4.2V per cell
- **Under-Voltage**: 2.6V per cell
- **Open-Wire (Cell)**: 2000mV deviation
- **Open-Wire (Aux)**: 50000mV deviation

### Fault Flags (SEG_BSTAT):
- Individual cell OV/UV detection
- Open-wire detection
- Communication errors
- IC temperature monitoring

### Safety Features:
- `AMS_SAFE` flag - Overall system safety status
- Discharge control via DCC registers
- Thermal shutdown detection
- PEC (Packet Error Code) validation

---

## Key Functional Flows

### 1. Initialization Sequence
```
main() 
├─ HAL_Init()
├─ SystemClock_Config()
├─ Initialize all peripheral drivers
├─ CAN_Data_Init()
├─ adbms_init_loop() 
│  ├─ Initialize ADBMS6830 IC
│  ├─ Set config registers (OV, UV, thresholds)
│  └─ Enable measurements
└─ HAL_TIM_Base_Start_IT(&htim3) - Start timer interrupt
```

### 2. Voltage Measurement Loop
```
adbms_readCell_loop()
├─ Trigger ADC_CONV on ADBMS IC
├─ Wait for conversion (typically 1-2ms)
├─ Read cell voltage registers via SPI
├─ Parse 6-byte data blocks per cell group
├─ Convert ADC codes to voltages
├─ Store in SEG_PARAMS.CELL_V[]
└─ Check voltage thresholds
```

### 3. Temperature Measurement
```
adbms_readTempToggle()
├─ Read auxiliary voltage registers (thermistor inputs)
├─ Convert voltage to temperature via thermistor curve
├─ Store in SEG_PARAMS.CELL_T[]
├─ Monitor IC internal temperature (IC_T)
└─ Check thermal shutdown condition
```

### 4. Diagnostic Tests (Optional)
```
testOpenWire()
├─ Enable open-wire detection algorithm
├─ Apply pull-up current to cells
├─ Measure voltage deviation
├─ Compare against OWC_Threshold
└─ Flag open wires if detected

adBms6830_run_*_self_test()
├─ Oscillator frequency verification
├─ Thermal shutdown functionality
├─ Supply voltage error detection
├─ Fuse integrity verification
└─ Report test results
```

---

## System States & Modes

### Operating Modes:
1. **Idle**: Minimal monitoring
2. **Normal**: Continuous cell voltage monitoring
3. **Charging**: Charge current tracking, load balancing via discharge
4. **Discharging**: Load distribution monitoring
5. **Fault**: Reduced functionality, error flagging
6. **Diagnostic**: Self-tests, open-wire detection

### Key Global Status Variables:
```c
AMS_SOC          // State of Charge (0-100%)
AMS_SAFE         // Safety flag (0=fault, 1=safe)
AMS_Charg_I      // Charging current (mA)
AMS_Charg_V      // Charging voltage (V)
TS_Current       // Total system current
MCU_junction_temp // Internal MCU temperature
```

---

## Communication Protocols

### SPI (Primary - ADBMS Control)
- **Clock**: ~1-2 MHz
- **Frame Format**: Command byte + address + data + PEC
- **Master**: STM32L552
- **Slave**: ADBMS6830 IC

### CAN-FD (Data Broadcast)
- **ID Range**: Standard CAN identifiers (11-bit)
- **Data Length**: 8-64 bytes
- **Update Rate**: 1Hz (1-second interval)
- **Consumers**: BMS Master ECU, Vehicle network

### UART (Debug Output)
- **Baud Rate**: 115200 bps
- **Connector**: LPUART1 (PG7-TX, PG8-RX)
- **Purpose**: Real-time data logging, diagnostics

---

## Code Organization Summary

```
L552_AMS/
├─ Core/
│  ├─ ADBMS6830/              # BMS IC driver module
│  │  ├─ inc/
│  │  │  ├─ adbms_main.h      # Main BMS routines
│  │  │  ├─ adBms_Application.h # Application interface
│  │  │  ├─ adBms6830Data.h   # Data structures & configs
│  │  │  ├─ adBms6830GenericType.h # Type definitions
│  │  │  ├─ adBms6830ParseCreate.h # Command builders
│  │  │  ├─ mcuWrapper.h      # Hardware abstraction
│  │  │  └─ serialPrintResult.h # Debug output
│  │  └─ src/
│  │     ├─ adBms_Application.c
│  │     ├─ adBms6830GenericType.c
│  │     ├─ adBms6830ParseCreate.c
│  │     ├─ mcuWrapper.c
│  │     └─ serialPrintResult.c
│  ├─ Inc/                    # Peripheral headers
│  │  ├─ main.h              # Main data structures
│  │  ├─ spi.h, i2c.h, gpio.h, etc.
│  │  └─ stm32l5xx_hal_conf.h # HAL configuration
│  └─ Src/                    # Peripheral implementations
│     ├─ main.c              # Main entry point
│     ├─ spi.c, i2c.c, gpio.c, etc.
│     ├─ system_stm32l5xx.c  # System initialization
│     └─ stm32l5xx_it.c      # Interrupt handlers
├─ Drivers/                   # STM32 HAL & CMSIS
└─ Debug/                     # Build artifacts
```

---

## Key Design Patterns

1. **Hardware Abstraction**: `mcuWrapper.h` isolates hardware-specific code
2. **Modular Configuration**: Measurement modes (ENABLED/DISABLED) allow compile-time tuning
3. **Global Data Sharing**: `SEG_PARAMS` and `SEG_BSTAT` used as global state
4. **Interrupt-Driven Timing**: TIM3 provides predictable measurement intervals
5. **CAN Broadcasting**: Decoupled data dissemination to vehicle network
6. **Error Resilience**: `adbmsReinitMain()` handles IC recovery

---

## Performance Characteristics

- **Measurement Rate**: ~330Hz (3ms main loop)
- **CAN Update Rate**: 1Hz (sent every 1 second)
- **Cell Count**: 60 total (5 segments × 12 cells)
- **Temperature Channels**: 12+ (1 IC thermistor + external)
- **Resolution**: ~0.1mV (cell voltage), ~0.1°C (temperature)
- **Voltage Range**: 0-5V (ADC input)

---

## Potential Areas for Extension

1. **Balancing Control**: Enable DCC discharge for cell balancing
2. **CAN Diagnostics**: CANopen/SAE J1939 implementation
3. **Redundancy**: Multiple ADBMS ICs for fault tolerance
4. **State Machine**: Formal FSM for charge/discharge modes
5. **Data Logging**: EEPROM/SD card storage of history
6. **Wireless**: Bluetooth connectivity for mobile monitoring
7. **AI/Predictive**: Battery health estimation, cycle counting

---

**Last Updated**: 23 February 2026
