/*
  uno_map.h - driver code for STM32F756 ARM processor on a Nucleo-F756ZG board

  Part of grblHAL

  Copyright (c) 2021 Terje Io

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#if N_ABC_MOTORS
#error "Axis configuration is not supported!"
#endif

#if TRINAMIC_ENABLE
#error "Trinamic plugin not supported!"
#endif

#define BOARD_NAME "Generic Uno"
#define I2C_PORT 1
#define SPI_PORT 3
#define IS_NUCLEO_BOB
#define VARIABLE_SPINDLE // Comment out to disable variable spindle

// Define step pulse output pins.
#define X_STEP_PORT             GPIOF // D2
#define X_STEP_PIN              15
#define X_STEP_BIT              (1<<X_STEP_PIN)
#define Y_STEP_PORT             GPIOE // D3
#define Y_STEP_PIN              13
#define Y_STEP_BIT              (1<<Y_STEP_PIN)
#define Z_STEP_PORT             GPIOF // D4
#define Z_STEP_PIN              14
#define Z_STEP_BIT              (1<<Z_STEP_PIN)
#define STEP_MASK               0
#define STEP_OUTMODE            GPIO_SINGLE

// Define step direction output pins.
#define X_DIRECTION_PORT        GPIOE // D5
#define X_DIRECTION_PIN         11
#define X_DIRECTION_BIT         (1<<X_DIRECTION_PIN)
#define Y_DIRECTION_PORT        GPIOE // D6
#define Y_DIRECTION_PIN         9
#define Y_DIRECTION_BIT         (1<<Y_DIRECTION_PIN)
#define Z_DIRECTION_PORT        GPIOF // D7
#define Z_DIRECTION_PIN         13
#define Z_DIRECTION_BIT         (1<<Z_DIRECTION_PIN)
#define DIRECTION_MASK          0
#define DIRECTION_OUTMODE       GPIO_SINGLE

// Define stepper driver enable/disable output pin.
#define STEPPERS_ENABLE_PORT   GPIOF // D8
#define STEPPERS_ENABLE_PIN    12
#define STEPPERS_ENABLE_BIT    (1<<STEPPERS_ENABLE_PIN)

// Define homing/hard limit switch input pins.
#define X_LIMIT_PORT            GPIOD // D9
#define X_LIMIT_PIN             15
#define X_LIMIT_BIT             (1<<X_LIMIT_PIN)
#define Y_LIMIT_PORT            GPIOD // D10
#define Y_LIMIT_PIN             14
#define Y_LIMIT_BIT             (1<<Y_LIMIT_PIN)
#ifdef VARIABLE_SPINDLE
  #define Z_LIMIT_PORT          GPIOA // D12
  #define Z_LIMIT_PIN           6
#else
  #define Z_LIMIT_PORT          GPIOA // D11
  #define Z_LIMIT_PIN           7
#endif
#define Z_LIMIT_BIT             (1<<Z_LIMIT_PIN)
#define LIMIT_MASK              (X_LIMIT_BIT|Y_LIMIT_BIT|Z_LIMIT_BIT) // All limit bits
#define LIMIT_INMODE            GPIO_SINGLE

// Define spindle enable and spindle direction output pins.
#ifdef VARIABLE_SPINDLE
  #define SPINDLE_ENABLE_PORT   GPIOA // on morpho header
  #define SPINDLE_ENABLE_PIN    15
#else
  #define SPINDLE_ENABLE_PORT   GPIOA // D12
  #define SPINDLE_ENABLE_PIN    6
#endif
#define SPINDLE_ENABLE_BIT      (1<<SPINDLE_ENABLE_PIN)
#define SPINDLE_DIRECTION_PORT  GPIOA // D13
#define SPINDLE_DIRECTION_PIN   5
#define SPINDLE_DIRECTION_BIT   (1<<SPINDLE_DIRECTION_PIN)

// Define spindle PWM output pin.
#ifdef VARIABLE_SPINDLE
#define SPINDLE_PWM_PORT        GPIOA // D11
#define SPINDLE_PWM_PIN         7
#define SPINDLE_PWM_BIT         (1<<SPINDLE_PWM_PIN)
#endif

// Define flood and mist coolant enable output pins.
#define COOLANT_FLOOD_PORT      GPIOF // A3
#define COOLANT_FLOOD_PIN       3
#define COOLANT_FLOOD_BIT       (1<<COOLANT_FLOOD_PIN)
#define COOLANT_MIST_PORT       GPIOF // A4
#define COOLANT_MIST_PIN        5
#define COOLANT_MIST_BIT        (1<<COOLANT_MIST_PIN)

// Define user-control CONTROLs (cycle start, reset, feed hold) input pins.
#define RESET_PORT              GPIOA
#define RESET_PIN               3 // A0
#define RESET_BIT               (1<<RESET_PIN)
#define FEED_HOLD_PORT          GPIOC
#define FEED_HOLD_PIN           0 // A1
#define FEED_HOLD_BIT           (1<<FEED_HOLD_PIN)
#define CYCLE_START_PORT        GPIOC
#define CYCLE_START_PIN         3 // A2
#define CYCLE_START_BIT         (1<<CYCLE_START_PIN)
#define CONTROL_MASK            (RESET_BIT|FEED_HOLD_BIT|CYCLE_START_BIT)
#define CONTROL_INMODE          GPIO_SINGLE

// Define probe switch input pin.
#define PROBE_PORT              GPIOF // A5
#define PROBE_PIN               10
#define PROBE_BIT               (1<<PROBE_PIN)

#define SD_CS_PORT              GPIOC
#define SD_CS_PIN               8
#define SD_CS_BIT               (1<<SD_CS_PIN)

/**/
