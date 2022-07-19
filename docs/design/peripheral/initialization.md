# How to Initialize Peripherals in emfrp-repl?
Initializing peripherals needs complex procedures.  
For example, VL53L0X (it is one of ToF sensor.) needs heavy procedures to initialize.  
The micropython code : https://github.com/uceeatz/VL53L0X/blob/master/VL53L0X.py  
programs complexly, including waiting, binary manipulations, timeout, and more...  
So we must choose the 2 ways below:
 * Prepare procedural programming language.
 * Implement it by C/C++, and enable and start to use by a special REPL command.

# Solutions 
## 1. Prepare procedural programming language
Not recommend.  

## 2. Implement initializations and enable by a special REPL command.
We can implement initializations by C/C++ easily, and prepare REPL commands.  
We need to work:
 1. Supply autoconf/make-like TUI or pretty formatted file to configure what peripherals to use.
 2. Prepare special REPL command like below:

'''
#use peripheral("VL53L0X", 500, 100)
'''

500 is timeout, 100 is sampling rate.

It should be able to choose timeout, pin number, and so on by user.

 3. We may also supply way of disabling peripherals.

e.g.
'''
#disuse peripheral("VL53L0X")
'''

So we may offer unique id to peripheral usage.

