Signal Generator I2C Protocol
=============================

During a signal I2C transmission, only one signal port (ie PYRT) may be either written to or read from. The choice was made to control each signal individually rather than in a single batch of four. Usually in normal flight, one or two signals are changing at a time (ex throttle and roll), so it would be inefficient to send the other, relatively stable values. This protocol allows reading from either previously sent values or from incoming radio signals. The latter case is particularly useful for debugging and logging signals during manual flight.

### Reading Pulse Length
Pulse lengths can be read from either memory or incoming radio signals. In the case of reading from memory, the most recently saved value will be returned. A single read message is one byte long, the bits of this byte correspond to the following functions:

| Bit | Function |
| --- | -------- |
| 0 | Read/Write (Must be 0) |
| 1 | Read source (0 for memory, 1 for radio)|
| 2/3 | Signal Port |
| 4-6 | alignment bits |
| 7 | Parity bit |

### Writing Pulse Length
An unused alignment bit is placed at bit 1 for the purpose of ensuring that the signal port is at the same location in either reading or writing mode. A single read message is two bytes long, the bits of these bytes correspond to the following functions:

| Bit | Function |
| --- | -------- |
| 0 | Read/Write (Must be 1)|
| 1 | alignment bit |
| 2/3 | Signal Port |
| 4-14 | Pulse length |
| 15 | Parity bit |



#### Signal Port Codes
Each of the four control signals, pitch, yaw, roll and throttle, may be read from or written to.

| Code | Port |
| ---- | ---- |
| 0b00 | Pitch |
| 0b01 | Yaw |
| 0b10 | Roll |
| 0b11 | Throttle |

#### Parity Bit
The final bit of either a reading or writing message is used to detect errors during the I2C transmission. The parity bit is set such that there are an **odd** number of high bits in a message, ie parity is set to 1 if there is an _even_ number of high bits in the message, and set to 0 if there are an _odd_ number of high bits. Note that although the alignment bits are not used explicitly to read or write pulses, they are factored into the parity calculations. Odd parity was chosen specifically to ensure that 0x00 and 0xFF would not be interpreted as valid messages, the correct form would be 0x01 and 0xFE respectively.


#### Example

All alignment bits are set to 0 for simplicity.

| Message | Meaning |
| ------- | ------- |
| 0x10 | Read yaw from memory |
| 0x61 | Read roll from radio transmitter |
| 0xBA 0x87 | Write 1347 microseconds to throttle |

