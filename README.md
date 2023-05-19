# ModbusTCP Server
Modbus TCP Server Implementation in C

## Description
The Modbus TCP Server is a program that allows communication with devices using the Modbus TCP protocol. Modbus is a widely used protocol in industrial automation for interfacing with various devices such as sensors, actuators, and programmable logic controllers (PLCs). This server implementation allows you to simulate a Modbus device and handle requests from Modbus clients.

It supports the following Modbus function codes:
- Read Coil Status (0x01)
- Read Holding Registers (0x03)
- Read Input Registers (0x04)
- Write Single Coil (0x05)
- Write Single Register (0x06)

The server listens for incoming connections and spawns a new thread for each client. It maintains a set of registers that can be read from or written to by the clients.


## Features

- Support for Modbus function codes 0x01, 0x03, 0x04, 0x05, and 0x06.
- Thread-based architecture to handle multiple client connections concurrently.
- Mutex-based synchronization for accessing shared registers.
- Basic error handling and reporting.

## Requirements

- C compiler
- pthread library

## Usage

1. Make sure you have a C compiler installed on your system.
2. Compile the server code using the following command:gcc server.c -o server -lpthread 
3. Run the server executable: ./server
4. The server will start listening on port 502, which is the default port for Modbus TCP.
