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
