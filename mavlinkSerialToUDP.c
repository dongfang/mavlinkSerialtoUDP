#include <stdint.h>
#include <pthread.h>

#define DEFAULT_BUFFER_SIZE 512
#define DEFAULT_PORT 57600
#define DEFAULT_BAUD 57600

typedef uint8_t bool;

typedef enum {
  NONE = 0,
  UDP,
  TCP
} linkType;

bool sendTextMessages;

/* 
 * True if MAVLink messages should be attempted interpreted (to align 
 * complete MAVLink messages to UDP messages and possibly also do text
 * logging of them)
 */
bool interpretMAVLink;

linkType mavlinkIPLink;
int mavlinkPort;

linkType textIPLink;
int textPort;

size_t bufferSize = DEFAULT_BUFFER_SIZE;

uint8_t* buffer;

void initSerialPort() {
}

void initMAVLinkSocket() {
}

void allocBuffer() {
  buffer = malloc(bufferSize);
  if (buffer == NULL) complain;
}

void receiveByte(uint8_t data) { 
}

bool shouldSendImmediately() {
  // buffer full or complete message or timeout.
  // Or better: buffer full or (complete message and buffer not ridiculously low) or timeout
  return true;
}

void mavlinkThread() {
  
}
