# AngryWindows

![whatever](https://github.com/ch3rn0byl/AngryWindows/blob/master/Images/Capture.PNG)

When you are trying to fuzz or exploit the kernel and your machine becomes sentient and starts building up saltiness from you bullying it all the time, this is what ends up happening. 
Eventually it'll start to defend itself and call you out, OR    

This is a driver that modifies the emoticon, color, and error messages of the Bluescreen of Death. This came about while I was working on something else and here it is! 

## How does this work?

The end goal was to ultimately resolve the unexported function `nt!BgpFwDisplayBugCheckScreen`. The only way I saw that would reliably get this function was to trace it back to an exported function. The only exported function that used this lead to `nt!KeBugCheckEx`.
I use `nt!MmGetSystemRoutineAddress` to resolve `nt!KeBugCheckEx` and then start disassembling KeBugCheckEx to pull out `nt!KeBugCheck2`. 

From KeBugCheck2, I began disassembling until I reach `nt!KiDisplayBlueScreen`. Lastly, I use KiDisplayBlueScreen to dynamically resolve `nt!BgpFwDisplayBugCheckScreen`. 

Everything needed to modify the Bluescreen of Death is inside BgpFwDisplayBugCheckScreen. The sad face emoticon is located at `nt!HalpPCIConfigReadHandlers+0x18`, which is the UNICODE_STRING datatype.
```
2: kd> ? nt!HalpPCIConfigReadHandlers+0x18 
Evaluate expression: -8769656301296 = fffff806`27c05910
2: kd> dt nt!_UNICODE_STRING fffff806`27c05910
 ":("
   +0x000 Length           : 4
   +0x002 MaximumLength    : 6
   +0x008 Buffer           : 0xfffff806`27c1faf4  ":(
```

All the messages pertaining to what is going on are all located inside of `nt!EtwpLastBranchLookAsideList`, beginning at offset 0x60. 
```
3: kd> ? nt!EtwpLastBranchLookAsideList + 0x60
Evaluate expression: -8769643397936 = fffff806`28853cd0
3: kd> dps fffff806`28853cd0 l2
fffff806`28853cd0  00000000`006a0068
fffff806`28853cd8  ffffb980`20ba4804
3: kd> dS fffff806`28853cd0
ffffb980`20ba4804  "Your device ran into a problem a"
ffffb980`20ba4844  "nd needs to restart."
```

If the strings are going to be modified, the length and the maximum buffer of the UNICODE_STRING _need to be adjusted_ to reflect the new value otherwise you will only get parts of your string. 

I've noticed a max buffer of around ~96 or so characters. If you go more than that, you will definitely overwrite the buffer in the percentage section. So instead of saying `complete`, you will get some of your characters in that data. 

## How to use
To use the driver, issue the following commands inside an elevated debugger: 
```
sc create <service name> binPath= <path to driver>/AngryWindows.sys type= kernel start= auto
sc start <service name>
```
This will create a service for the driver and start it automatically. Anyhow...

Have fun!
