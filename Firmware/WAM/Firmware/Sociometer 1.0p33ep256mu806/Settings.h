/* 
 * Copyright (c) 2009-2011, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

// Configuration settings, status and command handling
// Dan Jackson, 2011


#include "HardwareProfile.h"


typedef enum { RECORD_NONE, RECORD_SOUND, RECORD_ACCEL, RECORD_BOTH } recordData_t;
            

// Device settings for startup
typedef struct
{
	// Device 
    unsigned short deviceId;            // Device identifier (stored in ROM)
    unsigned long sessionId;            // Session identifier

	// Logging mode
    unsigned char sampleRate;           // Sample rate code
    unsigned long loggingStartTime;     // Date/time to start logging
    unsigned long loggingEndTime;       // Date/time to end logging
    unsigned long maximumSamples;       // Maximum number of samples to record
	union {	
		unsigned char all;
		struct {
			unsigned char accel : 1;
			unsigned char gyro : 1;
			unsigned char mag : 1;
			unsigned char alt : 1;
			unsigned char adc : 1;
			unsigned char unused1 : 1;
			unsigned char unused2 : 1;
			unsigned char unused3 : 1; 
		};}loggingEnable;

	// Bluetooth mode	
	unsigned char btEnable;				// Runs bluetooth before logging

	// Misc
    unsigned char debuggingInfo;        // Debug flashing of state (0 = off, 1 = constant, 2 = on tap)
    unsigned long lastChangeTime;       // Date/time of last metadata change
    short timeZone;                     // RTC timezone offset from UTC in minutes (-1 = unknown)

	// WAM-specific
    recordData_t recordData;
	unsigned long loggingHours;			// Bitmap of which hours to record audio; 0x00ffffff = 24 hours
#ifdef ENABLE_ENCRYPT
	#define KEY_SIZE 32
	char key[KEY_SIZE];					// Encryption key (NULL-terminated string), empty = do not encrypt
#endif
#ifdef ENABLE_SCRAMBLE
	unsigned char scramble;				// 0 = off, 1 = sector swizzle
	unsigned char dummy;				// 0 = off, 1 = dummy data mode (effectively 32 Hz sampling)
#endif
	unsigned short inactivity;			// Inactivity
	unsigned long proximity;
    unsigned char audioRate;            // 1 = 16kHz, otherwise 8kHz

	//char proxEnable;
	//char altAccel;

} Settings;

extern Settings settings;


// Device logging status
typedef struct
{
    // Status
    char attached;                      // USB attached (>0 = configured)
    unsigned short initialBattery;      // Battery level at start (used for battery health)
    
    // Status: logging
    unsigned long accelSequenceId;           // Sequence counter for logging
    unsigned long gyroSequenceId;           // Sequence counter for logging
    unsigned char events;               // Event flags
    unsigned long sampleCount;          // Total count of samples in the file
    unsigned short lastSampledTicks;    // Time last sampled enough for a sector
    unsigned short lastWrittenTicks;    // Time last successfully written
    unsigned char debugFlashCount;      // Remaining number of debug flashes

    // Status: attached
    unsigned short lockCode;            // Comms lock when non-zero
    char batteryFull;                   // Battery fully charged flag (set only)
    char ledOverride;                   // Set LED colour (-1 = automatic)
    char diskMounted;                   // USB disk mounted flag
    char stream;                        // Streaming data while connected
    char actionFlags;                   // Settings for delayed actions
    char actionCountdown;               // Countdown to delayed action
    
    unsigned short resetReason;			// Reset
    
	#ifdef OFFSET_LOGGING
		unsigned long filenameBCD;		// For logging
		unsigned long offset;
	#endif
    
} Status;

extern Status status;


// Logging format
//#define FORMAT_NONE            0x00
//#define FORMAT_CSV             0x01
//#define FORMAT_CWA_PACKED      0x02
//#define FORMAT_CWA_UNPACKED    0x04
//#define FORMAT_SINGLE_FILE     0x10
//#define FORMAT_MASK_TYPE       0x0f
//#define FORMAT_MASK_OPTIONS    0xf0



typedef enum { SETTINGS_BATCH, SETTINGS_USB } SettingsMode;

void SettingsInitialize(void);
char SettingsReadFile(const char *filename);
char SettingsCommand(const char *line, SettingsMode mode);


#define ACTION_FORMAT_WIPE     0x01
#define ACTION_FORMAT_QUICK    0x02
#define ACTION_DELETE          0x04
#define ACTION_SAVE            0x08
#define ACTION_CREATE          0x10
#define ACTION_REMOUNT         0x20
#define ACTION_RESTART         0x40

char SettingsAction(char flags);


#define LOG_SIZE 32                 // "*TTTT##########################0"
#define LOG_COUNT 15
/*
// Device history/log
typedef struct
{
    unsigned short restartCount;        // Restart count
    unsigned short rechargeCount;       // Number of recharge cycles
    unsigned short reserved[14];        // 28 bytes of reserved space

    char log[LOG_COUNT][LOG_SIZE];      // 15x 32-byte log chunks
} Log;
*/
#define LOG_VALUE_RESTART 0
#define LOG_VALUE_BATTERY 1

#define LOG_CATEGORY_USER 0x0100
#define LOG_CATEGORY_STOP 0x0200

unsigned short SettingsGetLogValue(unsigned int index);
unsigned short SettingsIncrementLogValue(unsigned int index);
void SettingsAddLogEntry(unsigned short status, unsigned long timestamp, const char *message);
const char *SettingsGetLogEntry(int index, unsigned short *status, unsigned long *timestamp);



// CWA constants
#define SETTINGS_FILE "SETTINGS.INI"
#define DEFAULT_FILE "DATA.OMX"     
#define LOG_FILE "LOG.TXT"

