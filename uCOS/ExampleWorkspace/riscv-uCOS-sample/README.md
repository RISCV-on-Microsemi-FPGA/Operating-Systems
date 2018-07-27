## uC/OS-II port for Microsemi RISC-V

### Test Platform and FPGA design:
This project is tested on following hardware platforms:

RISCV-Creative-Board
- [RISC-V Creative board Mi-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/RISC-V-Creative-Board/tree/master/Programming_The_Target_Device/PROC_SUBSYSTEM_MIV_RV32IMA_BaseDesign)

PolarFire-Eval-Kit
- [PolarFire-Eval-Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/PolarFire-Eval-Kit/tree/master/Programming_The_Target_Device/MIV_RV32IMA_L1_AHB_BaseDesign)

M2S150-Advanced-Dev-Kit
- [SmartFusion2 Advanced Development Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/M2S150-Advanced-Dev-Kit/tree/master/Programming_The_Target_Device/PROC_SUBSYSTEM_BaseDesign)

### How to run the uC/OS-II RISC-V port:
To know how to use the SoftConsole workspace, please refer the [Readme.md](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole/blob/master/README.md)

Top folders of riscv-uCOS-sample

`bsp`: Board Support Package source code for LED's and OS Tick timer

`drivers`: This folder contains DirectCore Soft IP CoreGPIO and CoreUARTapb drivers source code.

`hal and riscv_hal`: This folder provides RISC-V soft processor hardware abstraction layer(RISC-V HAL) boot code, interrupt handling and hardware access methods for Mi-V soft processor. The source code can also be downloaded from [Mi-V-Firmware](https://github.com/RISCV-on-Microsemi-FPGA/Mi-V-Firmware)

`Micrium`: uC/OS-II for RISC-V port source code
    
`Micrium_libgen`: This Folder contains library file of uC/OS-II. The application can call OS services(FLAGS, Mailboxes, Mutex, SoftwareTimer, semaphores.. etc) using this library and also find the API of uC/OS-II in documentation at [micrium website](https://doc.micrium.com/pages/viewpage.action?pageId=16879190)    


The riscv-uCOS-sample is a self contained project. This project demonstrates 
the uC/OS-II running with Microsemi RISC-V processor. This project creates  three 
tasks and runs them at regular intervals.
    
This example project requires USB-UART interface to be connected to a host PC. 
The host PC must connect to the serial port using a terminal emulator such as 
TeraTerm or PuTTY configured as follows:
    
        - 115200 baud
        - 8 data bits
        - 1 stop bit
        - no parity
        - no flow control
    
The ./hw_platform.h file contains the FPGA design related information that is required 
for this project. If you update the FPGA design, the hw_platform.h must be updated 
accordingly.
    
### uCOS Configurations
You must configure the uCOS as per your applications need. Please read and modify .\os_cfg.h,
.\cpu_cfg.h and .\lib_cfg.h.

E.g. You must use the processor clock and Memory size as per the Libero design that 
you are using.

The RISC-V creative board design uses 66Mhz processor clock. The PolarFire Eval Kit design uses 50Mhz processor clock. The SmartFusion2 Adv. Developement kit design uses 83Mhz processor clock.
   
### Microsemi SoftConsole Tool-chain:
To know more refer: [SoftConsole](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole)

### Documentation for Microsemi RISC-V processor, SoftConsole toochain, Debug Tools, FPGA design etc.
To know more refer: [Documentation](https://github.com/RISCV-on-Microsemi-FPGA/Documentation)
    
