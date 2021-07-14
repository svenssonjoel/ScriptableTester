# Scriptable Iot/Embedded system Tester (Work in progress)

The plan is to implement a system for providing stimuli to an embedded
system under test in a repeatable way while collecting statistics about 
for example current draw. 


## Required equipment 

1. Computer capable of running Qt applications with Bluetooth.
2. Ruideng UM25C ammeter.
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
2. Response time measurement 
   - Can we trigger two captures on a counter from two different GPIO? 
   - Can we set an output gpio to trigger capture?
   - How fast can we run the counter (TIMx)? (I think the answer is 84MHz which would give a period of 11.904761904762 ns) 
     

PA0 - PA3 can be connected to TIM5_CH1 - TIM5_CH4 using (alternate function) AF 2 

- Not sure timer_reset works. Atleast not as I expect. 
- Need to figure out how fast the timer is counting. More reading in the RM :/
