# Scriptable Iot/Embedded system Tester

Testing of IoT and embedded using cheap "off-the-shelf" hardware. 

The testing-system consists of an computer with QT and bluetooth (if using Bluetooth connected ammeter),
an STM32F4-DISC1 discovery board and an ammeter (UM25C, PMODISNS20 is work in progress, Other sensors will be added).

## Features (All currently work in progress and semi-working)

1. Graphing of current consumption.
2. Response time measurements.
3. Scriptable stimuli

## Response time testing

The response time testing uses an 84MHz timer on the STM32F4-DISC1 in capture mode.
The STM32 is configures to capture the timer value into two different registers given a positive flank
on two different GPIOs.

The STM32F4-DISC1 configures GPIO A0 and GPIO A1 as alternate mode 2, which connects these
GPIOs to TIM5_ch1 and TIM5_ch2 (It is interesting how GPIOs are numbered starting at index 0
but the TIM5 capture channels are numbered 1 - 4, instead of 0 - 3).

A Third GPIO on the STM32F4-DISC1 is used in output mode to initiate a request. This is GPIO A2. This
GPIO should be connected to the System Under Test (SUT) as well as to GPIO A0 on the STM32F4-DISC1. 

The response measurement work like this.
1. STM32F4-DISC1 sets GPIO A2 low and waits for A0 and A1 to both be LOW.
   When A0 and A1 are both low the capture registers are cleared and the timer is reset.
2. The STM32F4-DISC1 sets GPIO A2 high, since this GPIO is connected to GPIO A0 the timer
   hardware detects a rising edge and stores the timer value in a register. The A2 GPIO is
   also connected to the SUT, which is expected to respond by writing HIGH onto one of its
   GPIOs which is connected to the STM32F4-DISC1 A1 GPIO.
3. When the SUT writes HIGH on its response-pin, the STM32F4-DISC1 timer hardware detects
   the rising edge on GPIO A2 and stores the current timer value into another register.
   This also sets of an interrupt to notify software that both a request and a response
   value is present for processing.

The interrupt service routine sends a message to a mailbox. The message contains
the request timestamp and the reply timestamp. The thread that is running the response
test is waiting on this mailbox and as soon as it gets a mail it puts together a
string to communicate to the GUI via UART.


## One possible setup

1. Computer capable of running Qt applications with Bluetooth.
2. Ruideng UM25C ammeter. very low sample rate, 2 - 3Hz it seems.
3. STM32F4-Discovery to act as the stimuli-provider. 

### Connect UM25

1. `hcitool scan` 

```
> hcitool scan
Scanning ...
	aa:bb:cc:dd:ee:ff	UM25C
```

2. `rfcomm bind 0 aa:bb:cc:dd:ee:ff`

## Future work

