Train OS - Bare metal operating system running on Raspi.

Reference document in the comment:

ARM Manual - Now is called "ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition Issue C" on ARM website. Arroding to ARM website. "The original ARM v6 Architecture Reference Manual for the ARM11 cores (ARM DDI 0100I) is superseded by the ARMv7-AR Architecture Reference Manual which is now the definitive document for Applications and Real-Time variants of the v6 and v7 ARM Architecture, including all ARM11-class cores."

Arm1176jzfs Manual - Can be found on ARM websitel called "ARM1176JZF-S Technical Reference Manual".




For ttc mode, use reserved GPIO address for testing
~~~c
#define TTC_OUT 0x20200018
#define TTC_IN 0x20200024
#define TTC_Ready 0x20200030
~~~

