# Scriptable Iot/Embedded system Tester

Testing of IoT and embedded using cheap "off-the-shelf" hardware. 


## Required equipment (Currently)

1. Computer capable of running Qt applications with Bluetooth.
2. Ruideng UM25C ammeter. very low sample rate, 2 - 3Hz it seems.
3. STM32F4-Discovery to act as the stimuli-provider. 


## Connect UM25

1. `hcitool scan` 

```
> hcitool scan
Scanning ...
	aa:bb:cc:dd:ee:ff	UM25C
```

2. `rfcomm bind 0 aa:bb:cc:dd:ee:ff`

## Future work

1. Add support for additional meters. [this](https://reference.digilentinc.com/pmod/pmodisns20/start) is one option.
   The Ruideng UM25C is a definite weak-spot and should be replaced with something with much higher sample rate. 
2. Response time measurement 
   - Can we trigger two captures on a counter from two different GPIO? 
	 We can capture the counter value upon events on two different GPIO.
	 PA0 - PA3 can be connected to TIM5_CH1 - TIM5_CH4 using (alternate function) AF 2 
3. The counter seems to be operating at 84MHz. Need to somehow validate that this is 
   the case. 
