/***********************************************************************
 * Copyright (c) 2014-2016 Ioannis Charalampidis
 * Copyright (c) 2015 Simon Schulz - github.com/fishpepper
  Copyright © 2019 Jean Michault, Petr Pulc.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*************************************************************************/

#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "CCDebugger.h"

int cc_init() {
    if (wiringPiSetup() == -1) {
        printf("No wiring pi detected.\n");
        return 0;
    }

    // Prepare CC Pins
    pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_DD, OUTPUT);
    pinMode(PIN_RST, OUTPUT);
    digitalWrite(PIN_DC, LOW);
    digitalWrite(PIN_DD, LOW);
    digitalWrite(PIN_RST, LOW);

    // Prepare default direction
    cc_setDDDirection(INPUT);

    // Default CCDebug instruction set for CC254x
    instr[INSTR_VERSION] = 1;
    instr[I_HALT] = 0x40;
    instr[I_RESUME] = 0x48;
    instr[I_RD_CONFIG] = 0x20;
    instr[I_WR_CONFIG] = 0x18;
    instr[I_DEBUG_INSTR_1] = 0x51;
    instr[I_DEBUG_INSTR_2] = 0x52;
    instr[I_DEBUG_INSTR_3] = 0x53;
    instr[I_GET_CHIP_ID] = 0x68;
    instr[I_GET_PC] = 0x28;
    instr[I_READ_STATUS] = 0x30;
    instr[I_STEP_INSTR] = 0x58;
    instr[I_CHIP_ERASE] = 0x10;

    // We are active by default
    cc_active = true;
};

uint8_t cc_error() {
    return errorFlag;
}

void cc_setActive(uint8_t on) {
    // Reset error flag
    errorFlag = CC_ERROR_NONE;

    // Continue only if active
    if (on == cc_active) return;
    cc_active = on;

    if (on) {
        // Prepare CC Pins
        pinMode(PIN_DC, OUTPUT);
        pinMode(PIN_DD, OUTPUT);
        pinMode(PIN_RST, OUTPUT);
        digitalWrite(PIN_DC, LOW);
        digitalWrite(PIN_DD, LOW);
        digitalWrite(PIN_RST, LOW);

        // Default direction is INPUT
        cc_setDDDirection(INPUT);
    } else {
        // Before deactivating, exit debug mode
        if (inDebugMode)
            cc_exit();

        // Put everything in inactive mode
        pinMode(PIN_DC, INPUT);
        pinMode(PIN_DD, INPUT);
        pinMode(PIN_RST, INPUT);
        digitalWrite(PIN_DC, LOW);
        digitalWrite(PIN_DD, LOW);
        digitalWrite(PIN_RST, LOW);
    }
}

uint8_t cc_enter() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    // Reset error flag
    errorFlag = CC_ERROR_NONE;

    // Enter debug mode
    digitalWrite(PIN_RST, LOW);
    cc_delay(200);
    digitalWrite(PIN_DC, HIGH);
    cc_delay(3);
    digitalWrite(PIN_DC, LOW);
    cc_delay(3);
    digitalWrite(PIN_DC, HIGH);
    cc_delay(3);
    digitalWrite(PIN_DC, LOW);
    cc_delay(4);
    digitalWrite(PIN_RST, HIGH);
    cc_delay(200);

    // We are now in debug mode
    inDebugMode = 1;

    // Success
    return 0;
};

uint8_t cc_exit() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_RESUME]); // RESUME
    cc_switchRead(250);
    bAns = cc_read(); // debug status
    cc_switchWrite();

    inDebugMode = 0;

    return 0;
}

uint8_t cc_exec(uint8_t oc0) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_DEBUG_INSTR_1]); // DEBUG_INSTR + 1b
    cc_write(oc0);
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

uint8_t cc_exec2(uint8_t oc0, uint8_t oc1) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_DEBUG_INSTR_2]); // DEBUG_INSTR + 2b
    cc_write(oc0);
    cc_write(oc1);
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

uint8_t cc_exec3(uint8_t oc0, uint8_t oc1, uint8_t oc2) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_DEBUG_INSTR_3]); // DEBUG_INSTR + 3b
    cc_write(oc0);
    cc_write(oc1);
    cc_write(oc2);
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

uint8_t cc_execi(uint8_t oc0, unsigned short c0) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_DEBUG_INSTR_3]); // DEBUG_INSTR + 3b
    cc_write(oc0);
    cc_write((c0 >> 8) & 0xFF);
    cc_write(c0 & 0xFF);
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

unsigned short cc_getChipID() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    unsigned short bAns;
    uint8_t bRes;

    cc_write(instr[I_GET_CHIP_ID]); // GET_CHIP_ID
    cc_switchRead(250);

    bRes = cc_read(); // High order
    bAns = bRes << 8;
    bRes = cc_read(); // Low order
    bAns |= bRes;
    cc_switchWrite();

    return bAns;
}

unsigned short cc_getPC() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    unsigned short bAns;
    uint8_t bRes;

    cc_write(instr[I_GET_PC]); // GET_PC
    cc_switchRead(250);
    bRes = cc_read(); // High order
    bAns = bRes << 8;
    bRes = cc_read(); // Low order
    bAns |= bRes;
    cc_switchWrite();

    return bAns;
}

uint8_t cc_getStatus() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_READ_STATUS]); // READ_STATUS
    cc_switchRead(250);
    bAns = cc_read(); // debug status
    cc_switchWrite();

    return bAns;
}

uint8_t cc_resume() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_RESUME]); //RESUME
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

uint8_t cc_halt() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_HALT]); //HALT
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

uint8_t cc_step() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_STEP_INSTR]); // STEP_INSTR
    cc_switchRead(250);
    bAns = cc_read(); // Accumulator
    cc_switchWrite();

    return bAns;
}

uint8_t cc_getConfig() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_RD_CONFIG]); // RD_CONFIG
    cc_switchRead(250);
    bAns = cc_read(); // Config
    cc_switchWrite();

    return bAns;
}

uint8_t cc_setConfig(uint8_t config) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_WR_CONFIG]); // WR_CONFIG
    cc_write(config);
    cc_switchRead(250);
    bAns = cc_read(); // Config
    cc_switchWrite();

    return bAns;
}

uint8_t cc_chipErase() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    };
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t bAns;

    cc_write(instr[I_CHIP_ERASE]); // CHIP_ERASE
    cc_switchRead(250);
    bAns = cc_read(); // Debug status
    cc_switchWrite();

    return bAns;
}


void cc_delay(unsigned char d) {
    volatile unsigned char i = 50 * d;
    while (i--);
//    struct timespec tp={0,0};
//    tp.tv_nsec=40*d;
//    nanosleep(&tp,NULL);
}

uint8_t cc_write(uint8_t data) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t cnt;

    // Make sure DD is on output
    cc_setDDDirection(OUTPUT);

    // Sent uint8_ts
    for (cnt = 8; cnt; cnt--) {
        // First put data bit on bus
        if (data & 0x80)
            digitalWrite(PIN_DD, HIGH);
        else
            digitalWrite(PIN_DD, LOW);

        // Place clock on high (other end reads data)
        digitalWrite(PIN_DC, HIGH);

        // Shift & Delay
        data <<= 1;
        cc_delay(2);

        // Place clock down
        digitalWrite(PIN_DC, LOW);
        cc_delay(2);
    }

    return 0;
}

uint8_t cc_switchRead(uint8_t maxWaitCycles) {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }
    if (!inDebugMode) {
        errorFlag = CC_ERROR_NOT_DEBUGGING;
        return 0;
    }

    uint8_t cnt;
    uint8_t didWait = 0;

    // Switch to input
    cc_setDDDirection(INPUT);

    // Wait at least 83 ns before checking state t(dir_change)
    cc_delay(2);

    // Wait for DD to go LOW (Chip is READY)
    while (digitalRead(PIN_DD) == HIGH) {
        // Do 8 clock cycles
        for (cnt = 8; cnt; cnt--) {
            digitalWrite(PIN_DC, HIGH);
            cc_delay(2);
            digitalWrite(PIN_DC, LOW);
            cc_delay(2);
        }

        // Let next function know that we did wait
        didWait = 1;

        // Check if we ran out if wait cycles
        if (!--maxWaitCycles) {
            // If we are waiting for too long, we have lost the chip,
            // so also assume we are out of debugging mode
            errorFlag = CC_ERROR_NOT_WIRED;
            inDebugMode = 0;

            return 0;
        }
    }

    // Wait t(sample_wait)
    if (didWait) cc_delay(2);

    return 0;
}

uint8_t cc_switchWrite() {
    cc_setDDDirection(OUTPUT);
    return 0;
}

uint8_t cc_read() {
    if (!cc_active) {
        errorFlag = CC_ERROR_NOT_ACTIVE;
        return 0;
    }

    uint8_t cnt;
    uint8_t data = 0;

    // Switch to input
    cc_setDDDirection(INPUT);

    // Send 8 clock pulses if we are HIGH
    for (cnt = 8; cnt; cnt--) {
        digitalWrite(PIN_DC, HIGH);
        cc_delay(2);

        // Shift and read
        data <<= 1;
        if (digitalRead(PIN_DD) == HIGH)
            data |= 0x01;

        digitalWrite(PIN_DC, LOW);
        cc_delay(2);
    }

    return data;
}

uint8_t cc_updateInstructionTable(const uint8_t newTable[16]) {
    // Copy table entries
    for (uint8_t i = 0; i < 16; i++)
        instr[i] = newTable[i];
    // Return the new version
    return instr[INSTR_VERSION];
}

uint8_t cc_getInstructionTableVersion() {
    // Return version of instruction table
    return instr[INSTR_VERSION];
}

void cc_setDDDirection(uint8_t direction) {
    // Switch direction if changed
    if (direction == ddDirection) return;

    // Handle new direction
    digitalWrite(PIN_DD, LOW); // Switch to low
    pinMode(PIN_DD, direction);   // Set direction
    digitalWrite(PIN_DD, LOW); // Switch to low

    ddDirection = direction;
}
