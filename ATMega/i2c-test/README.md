ATMega328P I2C Test
===================

Quick test program to learn I2C communication between the AVR and the RasPi. The
Raspberry Pi will be used as a Master and the ATMega will be used as a slave. In
the current test program, the master will send a byte telling the slave to set
a GPIO pin to either HIGH or LOW. The slave device will then send back a byte as
a confirmation of the GPIO write.

### Master:

Send 0x12 to turn GPIO on

Send 0x34 turn GPIO off

### Slave (not implemented yet):

Send 0x01 when pin is set to HIGH

Send 0x00 when pin is set to LOW


In the final implementation, the master will send a tell the slave to store a
pulse length in one of four registers, corresponding to the pitch, yaw, roll,
and throttle signals. The master can also send a read request of any of these
registers as well as a status request.

---

For more information on the ATMega's I2C interface, refer to Chapter 22 of the
ATMega328P Datasheet. For a description of registers used in I2C, see section 22.9
