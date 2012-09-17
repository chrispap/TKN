TKN
===
A network of AVR MCUs.

**Description**
- The protocol used is a custom Token Ring
- The goal is to interconnect a number of AVR based 
microcontroller nodes to a real time network so that 
any of them is able to send data to any of them.

**Physical layer**
- As the network's physical layer, the standard serial port is used.

**Topology**
- Every node of the network connects its Tx pin to the next node's Rx. 
- The last node's Tx is connected to the first node's Rx.
- In that way the ring is formed.

The protocol has been implemented both for AVR nodes and for PC so that 
a desktop application can communicate with the microcontrollers.

There are sample programs for the PC 
with command line or graphical interface to 
the network. _???_

**Source Layout**
`   

├── AVR          Protocol implementation for 8bit AVR MCUs (AVR Assembly)   

└── PC           Code for the PC   

    ├── console   Command line programs (C)   

    ├── gui       Graphical program (C++, Qt)   

    └── lib       Protocol implementation for PC, portable to windows/linux (C)   
    
    
    
`
