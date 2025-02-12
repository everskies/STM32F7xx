/*
  reference_map.h - driver code for STM32F7561 ARM processor on a Nucleo-F756 board

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

#if N_ABC_MOTORS > 1
#error "Axis configuration is not supported!"
#endif

#if TRINAMIC_ENABLE
#error "Trinamic plugin not supported!"
#endif

//#error "Reference map is work in progress - pin assignments are subject to changes, do not use!"

#define BOARD_NAME "grblHAL reference map"
#define HAS_IOPORTS
#define I2C_PORT 1
#define SPI_PORT 3
#define IS_NUCLEO_BOB
#define VARIABLE_SPINDLE // Comment out to disable variable spindle

// Define stepper driver enable/disable output pin.
#define STEPPERS_ENABLE_PORT   GPIOF
#define STEPPERS_ENABLE_PIN    12
#define STEPPERS_ENABLE_BIT    (1<<STEPPERS_ENABLE_PIN)

// Define step pulse output pins.
#define X_STEP_PORT         GPIOE
#define X_STEP_PIN          10
#define X_STEP_BIT          (1<<X_STEP_PIN)
#define Y_STEP_PORT         GPIOE
#define Y_STEP_PIN          12
#define Y_STEP_BIT          (1<<Y_STEP_PIN)
#define Z_STEP_PORT         GPIOE
#define Z_STEP_PIN          14
#define Z_STEP_BIT          (1<<Z_STEP_PIN)
#define STEP_OUTMODE        GPIO_SINGLE
#define STEP_MASK 0

// Define step direction output pins.
#define X_DIRECTION_PORT    GPIOE
#define X_DIRECTION_PIN     11
#define X_DIRECTION_BIT     (1<<X_DIRECTION_PIN)
#define Y_DIRECTION_PORT    GPIOE
#define Y_DIRECTION_PIN     9
#define Y_DIRECTION_BIT     (1<<Y_DIRECTION_PIN)
#define Z_DIRECTION_PORT    GPIOE
#define Z_DIRECTION_PIN     13
#define Z_DIRECTION_BIT     (1<<Z_DIRECTION_PIN)
#define DIRECTION_OUTMODE   GPIO_SINGLE

// Define homing/hard limit switch input pins.
#define X_LIMIT_PORT        GPIOD
#define X_LIMIT_PIN         15
#define X_LIMIT_BIT         (1<<X_LIMIT_PIN)
#define Y_LIMIT_PORT        GPIOD
#define Y_LIMIT_PIN         14
#define Y_LIMIT_BIT         (1<<Y_LIMIT_PIN)
#ifdef VARIABLE_SPINDLE
  #define Z_LIMIT_PORT      GPIOA
  #define Z_LIMIT_PIN       6
#else
  #define Z_LIMIT_PORT      GPIOA
  #define Z_LIMIT_PIN       7
#endif
#define Z_LIMIT_BIT         (1<<Z_LIMIT_PIN)
#define LIMIT_INMODE        GPIO_SINGLE

#if N_ABC_MOTORS == 1
#define M3_AVAILABLE
#define M3_STEP_PORT            GPIOE
#define M3_STEP_PIN             0
#define M3_DIRECTION_PORT       GPIOF
#define M3_DIRECTION_PIN        14
#define M3_LIMIT_PORT           GPIOB
#define M3_LIMIT_PIN            10
#ifndef STEPPERS_ENABLE_PORT
#define M3_ENABLE_PORT          GPIOE
#define M3_ENABLE_PIN           14
#endif
#endif

// Define spindle enable and spindle direction output pins.
#ifdef VARIABLE_SPINDLE
  #define SPINDLE_ENABLE_PORT   GPIOA // on morpho header
  #define SPINDLE_ENABLE_PIN    15
#else
  #define SPINDLE_ENABLE_PORT   GPIOA
  #define SPINDLE_ENABLE_PIN    6
#endif
#define SPINDLE_ENABLE_BIT      (1<<SPINDLE_ENABLE_PIN)
#define SPINDLE_DIRECTION_PORT  GPIOA
#define SPINDLE_DIRECTION_PIN   5
#define SPINDLE_DIRECTION_BIT   (1<<SPINDLE_DIRECTION_PIN)

// Define spindle PWM output pin.
#ifdef VARIABLE_SPINDLE
#define SPINDLE_PWM_PORT        GPIOA
#define SPINDLE_PWM_PIN         7
#define SPINDLE_PWM_BIT         (1<<SPINDLE_PWM_PIN)
#endif

// Define flood and mist coolant enable output pins.
#define COOLANT_FLOOD_PORT      GPIOF
#define COOLANT_FLOOD_PIN       3
#define COOLANT_FLOOD_BIT       (1<<COOLANT_FLOOD_PIN)
#define COOLANT_MIST_PORT       GPIOF
#define COOLANT_MIST_PIN        5
#define COOLANT_MIST_BIT        (1<<COOLANT_MIST_PIN)

// Define user-control CONTROLs (cycle start, reset, feed hold) input pins.
#define RESET_PORT              GPIOA
#define RESET_PIN               3
#define RESET_BIT               (1<<RESET_PIN)
#define FEED_HOLD_PORT          GPIOC
#define FEED_HOLD_PIN           0
#define FEED_HOLD_BIT           (1<<FEED_HOLD_PIN)
#define CYCLE_START_PORT        GPIOC
#define CYCLE_START_PIN         3
#define CYCLE_START_BIT         (1<<CYCLE_START_PIN)
#define CONTROL_MASK            (RESET_BIT|FEED_HOLD_BIT|CYCLE_START_BIT)
#define CONTROL_INMODE          GPIO_SINGLE

// Define probe switch input pin.
#define PROBE_PORT              GPIOF
#define PROBE_PIN               10
#define PROBE_BIT               (1<<PROBE_PIN)

#define SD_CS_PORT              GPIOC
#define SD_CS_PIN               8
#define SD_CS_BIT               (1<<SD_CS_PIN)

#define AUXINPUT0_PORT          GPIOE
#define AUXINPUT0_PIN           15
#define AUXINPUT1_PORT          GPIOB
#define AUXINPUT1_PIN           10
#define AUXINPUT2_PORT          GPIOE
#define AUXINPUT2_PIN           2
#define AUXINPUT_MASK           (1<<AUXINPUT0_PIN|1<<AUXINPUT1_PIN|1<<AUXINPUT2_PIN)

#define AUXOUTPUT0_PORT         GPIOB
#define AUXOUTPUT0_PIN          11

/**/
