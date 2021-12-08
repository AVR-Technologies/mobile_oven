# init
- init controller for eeprom data

# controller
- user input using buttons (on config page)
    1. fog timer control (row1 on lcd)
    2. oven timer control (row2 on lcd)
    3. machine cycle display (row3 on lcd)
- lcd ui
- manual lcd print

# controller 1
- fogRelay and ovenRelay control
- use char array instead of manual lcd print
- eeprom settings save and retrive

# controller 2
- take oven temp (on config page) 
    1. ~~machine cycle display~~ oven temperature (row3 on lcd) 
- maintain oven temperature after oven cycle start
- save and retrieve oven temp config from eeprom

# controller 3
- remove temperature from fog screen
- temperature cutoff threshold: ~~5~~ 10
- temperature max limit: ~~80~~ 40
- config screen
    - remote title
    - display no. of cycle
    - press and hold (+) button to reset cycles.
    - now using enum (EditIndex) (see on constants.h) instead of int to shift between edit parameters.
- added select screen after home screen to choose
    - fog timer
    - oven timer
- after fog timer and oven timer completion display complete screen
- increament cycle after oven timer complete