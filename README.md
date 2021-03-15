halpp
==

Just another C++ hardware abstraction Layer :sweat_smile:

:warning: Experimental, use at your own risk.

The only currently supported MCU is STM32F103, but support for STM32F405, STM32F030, and ATmega328P probably will be implemented.

:warning: Tested with BlackPill F103 devboard.

Feature status
==

| Target    | F030 | F103      | F401 | F405 | F407 | F429 | ATmega328p |
|-----------|------|-----------|------|------|------|------|------------|
|Compiles   | :white_check_mark:  |:question: |  :x: | :x:  | :x:  | :x:  | :question: |
|Blinks LED | :white_check_mark:  |:question: | :x:  | :x:  | :x:  | :x:  | :question: |
|Delay works| :question:  |:question: | :x:  | :x:  | :x:  | :x:  | :x:        |
|Interrupts | :x: | :question: | :x: | :x: | :x: | :x:| :x: |