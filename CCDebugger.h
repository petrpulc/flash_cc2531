#ifndef CCDEBUGGER_H
#define CCDEBUGGER_H

#define CC_ERROR_NONE           0
#define CC_ERROR_NOT_ACTIVE     1
#define CC_ERROR_NOT_DEBUGGING  2
#define CC_ERROR_NOT_WIRED      3

/**
 * Software-overridable instruction table that can be used
 * for supporting other CCDebug-Compatible chips purely by software
 */
uint8_t instr[16];

/**
 * Instruction table indices
 */
#define INSTR_VERSION   0
#define I_HALT          1
#define I_RESUME        2
#define I_RD_CONFIG     3
#define I_WR_CONFIG     4
#define I_DEBUG_INSTR_1 5
#define I_DEBUG_INSTR_2 6
#define I_DEBUG_INSTR_3 7
#define I_GET_CHIP_ID   8
#define I_GET_PC        9
#define I_READ_STATUS   10
#define I_STEP_INSTR    11
#define I_CHIP_ERASE    12

#define I_SET_HW_BRKPNT 13
#define I_GET_BM        14
#define I_BURST_WRITE   15

/**
 * Local properties
 */
int pinRST = 24;
int pinDC = 28;
int pinDD = 29;
uint8_t errorFlag = 0;
uint8_t ddIsOutput = false;
uint8_t inDebugMode = false;
uint8_t cc_active = false;


////////////////////////////
/// High-Level interaction
////////////////////////////

/**
 * Initialize debugger
 */
int cc_init(int pinRST, int pinDC, int pinDD);

/**
 * Return error code
 */
uint8_t cc_error();

/**
 * Activate/Deactivate debugger
 */
void cc_setActive(uint8_t on);

/**
 * Enter debug mode
 */
uint8_t cc_enter();

/**
 * Exit from debug mode
 */
uint8_t cc_exit();

/**
 * Invoke a debug instruction with 1 opcode
 */
uint8_t cc_exec(uint8_t oc0);

/**
 * Invoke a debug instruction with 2 opcodes
 */
uint8_t cc_exec2(uint8_t oc0, uint8_t oc1);

/**
 * Invoke a debug instruction with 3 opcodes
 */
uint8_t cc_exec3(uint8_t oc0, uint8_t oc1, uint8_t oc2);

/**
 * Invoke a debug instruction with 1 opcode + 16-bit immediate
 */
uint8_t cc_execi(uint8_t oc0, unsigned short c0);

/**
 * Return chip ID
 */
unsigned short cc_getChipID();

/**
 * Return PC
 */
unsigned short cc_getPC();

/**
 * Return debug status
 */
uint8_t cc_getStatus();

/**
 * Resume program
 */
uint8_t cc_resume();

/**
 * Halt program
 */
uint8_t cc_halt();

/**
 * Step a single instruction
 */
uint8_t cc_step();

/**
 * Get debug configuration
 */
uint8_t cc_getConfig();

/**
 * Set debug configuration
 */
uint8_t cc_setConfig(uint8_t config);

/**
 * Mass-erase all chip configuration & Lock Bits
 */
uint8_t cc_chipErase();

////////////////////////////
/// Low-level interaction
////////////////////////////

/**
 * Delay a particular number of cycles
 */
void cc_delay(unsigned char d);

/**
 * Write a uint8_t to the debugger
 */
uint8_t cc_write(uint8_t data);

/**
 * Wait until we are ready to read & Switch to read mode
 */
uint8_t cc_switchRead(uint8_t maxWaitCycles);

/**
 * Switch to write mode
 */
uint8_t cc_switchWrite();

/**
 * Read a uint8_t from the debugger
 */
uint8_t cc_read();

/**
 * Update the debug instruction table
 */
uint8_t cc_updateInstructionTable(uint8_t newTable[16]);

/**
 * Get the instruction table version
 */
uint8_t cc_getInstructionTableVersion();

/**
 * Set DD pin direction
 */
void cc_setDDDirection(uint8_t direction);

#endif

