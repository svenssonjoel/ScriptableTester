# Attempt to interface with um25



## Connect UM25

1. `hcitool scan` 

```
> hcitool scan
Scanning ...
	aa:bb:cc:dd:ee:ff	UM25C
```

2. `rfcomm bind 0 aa:bb:cc:dd:ee:ff`


