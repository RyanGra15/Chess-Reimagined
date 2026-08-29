# Chess-Reimagined

An electronic chess board, that you can play (and lose) against 

## Features

* 64 Hall effect sensors to detect the position of the chess pieces
* LEDs matrix to indicate moves
* 0.96 inch OLED screen
* 3D printed board housing

## How It Works

The board uses Hall effect sensors underneath each square to detect whether a chess piece is present. Each chess piece will have a magnet embedded into its base, allowing the sensor underneath it to detect that a piece is present. Then, the software will take this, and represent it as a 64 bit string, with each bit signifying if there is a piece on the corresponding square. 

By backtracking through prior positions, all the way to the starting position, the ESP32 will be able to determine where each piece is. Then over Wifi, it will call the Stockfish API, get the best move, convert it back to the 64 bit string, and send it back to the Arduino. 

The Arduino will then control the LEDs underneath the board. The two squares involved in the move will light up, showing the player which piece to move and where to move it.

## Electronics

I am using multiplexing-esque approach for both the Hall effect sensors and the LEDs. By feeding 5V to a specified row, and making a specified column GND, I can choose to give power to a specific part. This allows me to control many signals without needing a separate connection for every single component. This keeps the circuit relatively simple and reduces the number of connections needed.

The main electronics are:

* Arduino Mega 2560
* ESP32
* Hall effect sensors
* LEDs
* S8550 PNP transistors
* Resistors
* Magnets
* Wires

## CAD Model

The board housing will be 3D printed using PLA+. I chose PLA+ because it is cost efficient and is available in many different colors. The board will use a light grey and cold white colour scheme. 

The squares on the chess board have a hole in which the LED can be slotted into, along with a separate inset, wherein a extremely weak magnet can be placed, so that the chess pieces do not repel or attract each other, and instead they attract to the magnets embedded in the base of each square. This also has the added upside that you can flip the board upside down (if that is more your style ;)). See image below:


<img width="666" height="502" alt="image" src="https://github.com/user-attachments/assets/b5ab69e3-76d2-42d4-a5d3-47cc8ec0b5db" />


The chess pieces are also designed digitally and 3D printed, with some inspiration taken from previous chess related projects. 


<img width="808" height="553" alt="image" src="https://github.com/user-attachments/assets/e18d7ed4-d06a-4e07-84e4-1590cef6c61d" />

The different parts will be attached together using hopes, dreams, and a lot of hot glue.

