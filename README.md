TKN
===
**Description**
 - A real time network protocol based on serial data transmission over a UART.
 - A network bootloader for AVR ATmega microcontrollers based on the above protocol.
 - A GUI application serving as user interface to the bootloader.

---
**Protocol implemented for**
 * AVR ATmega microcontrollers
 * PC (Linux, Win32)

---
**Network Topology**
- Every node of the network connects its Tx pin to the next node's Rx. 
- The last node's Tx is connected to the first node's Rx.
- In that way the ring is formed.

---
**Source Layout**
* AVR
   * **TKN.asm**     Protocol implementation for AVR ATmega microcontrollers _(AVR Assembly)_
   * **TKNBoot.asm** Bootloader code _(AVR Assembly)_
   * **Nodes**       Small independent sample programs _(AVR Assembly)_
* PC
   * **lib**     Protocol implementation for PC _(C - Linux, Win32)_
   * **console** Command line applications demonstrating the usage of the protocol _(C - Linux, Win32)_
   * **gui**     Gui application offering a console interface to the network nodes as long as the capability to upload
                 code to them via the Bootloader _(C++, Qt - Linux, Win32)_

