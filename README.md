# Mini-Project-IS1200

The project aims to recreate the feel of a classic arcade game machine using the DE10-Lite board. The code handles real-time interaction through hardware components such as LEDs, switches, seven-segment displays, and a VGA output, all controlled directly via memory-mapped I/O. It implements two mini-games (inspired by humanbenchmark.com): Memory Game and Reaction Game. In the first one, you must recall image sequences, while the second tests your reaction time to a changing screen. Behind the scenes, interrupts, timers, and pixel buffer logic work together to deliver a smooth and user-friendly interface, entirely powered by custom C logic running directly on the board.

### Features
- Two interactive mini-games: **Memory Game** and **Reaction Game**
- Real-time hardware interaction through **switches**, **LEDs**, and **button input**
- Dynamic **VGA output** for displaying images and animations
- Uses **interrupts**, **timers**, and **performance counters** for smooth execution
- Modular C structure for easy maintenance and expansion

### Build and Run
1. Open a **Linux environment** or a compatible **virtual machine**.  
2. Connect the **DE10-Lite board** via USB.  
3. Connect the board to an output monitor through a **VGA** cable.
4. Upload the raw pictures to the board by running `cd raw` followed by `./load.sh` in the terminal.
5. Compile the code using `make`:
6. Run the program with `dtekv-run main.bin`.

### Usage 
- In the main menu switch #0 (from the right) is used to access Memory Game and switch #3 for Reaction Game.
- Press button to select the game.
- For each mini-game, a tutorial will display the rules.
- At (almost) any point of the program, turning on only switch #9 (the leftmost one) will return you to the main menu.

### Authors
Developed by **Francesco Camporeale** & **Vittorio Fiammenghi**  
IS1200 Mini Project — KTH Royal Institute of Technology


