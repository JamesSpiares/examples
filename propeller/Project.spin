'CSCE 3612 Project Solution
'Author: James Spiares
'Connections:
'
'  Pin 3 connected to LED 15,  Pin 4 connected to LED 14,  Pin 5 connected to LED 13;  indicate machine 1
'  Pin 6 connected to LED 11,  Pin 7 connected to LED 10,  Pin 8 connected to LED 9;   indicate machine 2
'  Pin 9 connected to LED 7,   Pin 10 connected to LED 6,  Pin 11 connected to LED 5;  indicate machine 3
'  Pin 12 connected to LED 3,  Pin 13 connected to LED 2,  Pin 14 connected to LED 1;  indicate machine 4
'  indicate roller             indicate cutter             indicate roll replacement
'
'   and
'  Parallax Serial Terminal connected to computer

CON
  
  _clkmode = xtal1 + pll16x                                                     'Clock constants
  _xinfreq = 5_000_000

  NUM_CYCLES = 2                                                                'The number of cycles to run each roller before the program ends                                                                                       
  ROLL_SPEED = 3                                                                'The speed at which the roller rolls fabric onto the cutting board, in m/s
  CUT_TIME   = 1                                                                'The time it takes the cutting device to make a cut, in s
  REPLACE_TIME = 3                                                              'The time it takes to replace a roll of fabric, in s
  ROLL_LENGTH = 100                                                             'The length of each roll of fabric, in m
  BAUD_RATE = 115200

VAR

  long ids[4]                                                                   'An array of id, 1-4, one of which is given to each runMachine instance
  long lock                                                                     'The id of the lock used to control access to the serial terminal
  byte stackSpace1[128]                                                         'The stack space for the each instance of runMachine, respectively
  byte stackSpace2[128]                                                      
  byte stackSpace3[128]                                                     
  byte stackSpace4[128]                                                   
   
OBJ

  pst:  "Parallax Serial Terminal"                                              'Serial terminal used to get input values and report output values
  
PUB main

  dira[3..14]~~                                                                 'Set pins to output
  repeat lock from 1 to 4                                                       'Initialize ids to 1-4
    ids[lock - 1] := lock                            

  lock := locknew                                                               'Initialize the lock

  pst.start(BAUD_RATE)                                                          'Start the serial terminal

  cognew(runMachine(ids[0], lock), @stackSpace1)                                'Start each instance of runMachine
  cognew(runMachine(ids[1], lock), @stackSpace2)
  cognew(runMachine(ids[2], lock), @stackSpace3)
  cognew(runMachine(ids[3], lock), @stackSpace4)
  
  repeat                                                                        'Keep this cog alive
    waitcnt(cnt + clkfreq * 1000)

PUB runMachine(id, lockid) | runtime, left, replacements, totaltime, time, start, length, cuts, waste, counter                  'Emulates control of one cutting machine

  dira[3..14]~~                                                                 'Set pins to output
  left := ROLL_LENGTH                                                           'Initialize length of roll remaining for this machine to length of roll                                                                                           '
  totaltime := 0                                                                'Initialize total time taken by this machine to 0
  replacements := 0                                                             'Initialize total number of replacements made by this machine to 0
  cuts := 0                                                                     'Initialize total number of cuts made by this machine to 0
  waste := 0                                                                    'Initialize total length of fabric wasted by this machine to 0

  waitcnt(cnt + clkfreq * (cogid + 1))                                          'Wait a number of seconds proportional to the cogid so that user is prompted in order 1-4
   
  repeat counter from 1 to NUM_CYCLES                                           'Perform NUM_CYCLES operating cycles
    
    if counter == 1                                                             'If this is the first cycle, gain access to terminal. In later cycles, it will already have access from the bottom of this loop (see line 129).
      repeat while lockset(lockid)
    pst.str(string("Enter the length of cuts for roller #"))                    'Prompt user for length of cuts
    pst.dec(id)                                                               
    pst.str(string(":  "))
    waitcnt(cnt + clkfreq / 10)
    length := pst.decin                                                         'Store user input
    waitcnt(cnt + clkfreq / 10)
    pst.str(string("Enter the minimum time for roller #"))                      'Prompt user for minimum running time
    pst.dec(id)
    pst.str(string(" to run:  "))
    waitcnt(cnt + clkfreq / 10)
    time := pst.decin                                                           'Store user input
    pst.str(string("Roller #"))                                                 'Indicate that the machine has started.
    pst.dec(id)
    pst.str(string(" is running."))
    pst.char(13)
    waitcnt(cnt + clkfreq / 2)
    lockclr(lockid)                                                             'Release access to terminal
   
    runtime := 0                                                                'Initialize runtime of this machine during this cycle to 0
    repeat while runtime < time                                                 'Run machine for at least the minimum runtime
      start := cnt                                                              'Store starting time of this cycle
      
      dira[3 * id] := 1                                                         'Emulate rolling fabric onto cutting board
      outa[3 * id] := 1
      waitcnt(cnt + clkfreq * length / ROLL_SPEED)
      outa[3 * id] := 0
    
      dira[3 * id + 1] := 1                                                     'Emulate cutting fabric
      outa[3 * id + 1] := 1
      waitcnt(cnt + clkfreq * CUT_TIME)
      cuts++                                                                    'Increment total number of cuts
      outa[3 * id + 1] := 0
   
      left -= length                                                            'Decrement length of fabric remaining by amount that was just cut off
   
      if (left < length)                                                        'If there is not enough fabric on the roll to make another cut...
        dira[3 * id + 2] := 1                                                   '...emulate replacing the roll
        outa[3 * id + 2] := 1
        waitcnt(cnt + clkfreq * REPLACE_TIME)
        replacements++                                                          'Increment the total number of replacements
        waste += left                                                           'Increment the total length of wasted fabric by the amount that was left on the roll
        left := ROLL_LENGTH                                                     'Re-initialize length of fabric left to the length of a new roll
        outa[3 * id + 2] := 0
   
      totaltime += ((cnt - start) / clkfreq)                                    'Increment runtime during this cycle by time taken to make last cut (and replace roll, if necessary)
      runtime += ((cnt - start) / clkfreq)                                      'Increment total runtime by the same amount
      
    repeat while lockset(lockid)                                                'Gain access to the terminal
    pst.str(string("Roller #"))                                                 'Report statistics to user
    pst.dec(id)
    pst.str(string(" was replaced "))
    pst.dec(replacements)
    pst.str(string(" times"))
    pst.char(13)
    pst.str(string("and ran for "))
    pst.dec(totaltime)
    pst.str(string(" seconds."))
    pst.char(13)
    pst.dec(cuts)
    pst.str(string(" cuts were made and "))
    pst.char(13)
    pst.dec(waste)
    pst.str(string(" m of fabric were wasted."))
    pst.char(13)
    waitcnt(cnt + clkfreq / 2)                                                  
    if counter == NUM_CYCLES                                                    'If this is the last cycle, release access to the terminal
      lockclr(lockid)                                                           'If it is not, keep access so it can prompt user at the top of this loop (see line 61).
  