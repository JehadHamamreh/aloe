[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/fbe47a2652453cdc1eb50219b38ab2f0 "githalytics.com")](http://githalytics.com/flexnets/aloe)
ALOE stands for Abstraction Layer and Open Operating Environment. It is an Open Source framework for distributed real-time signal processing for SDR (Software-Defined Radio) applications. ALOE is released under the LGPL license (see license.txt) 

ALOE++ is partially supported by the NLnet foundation (http://www.nlnet.nl), as part of the OSLD project. OSLD aims at building an Open Source LTE system based on ALOE. More information and documentation can be found in the OSLD project website (https://sites.google.com/site/osldproject/)

[Click here to know more about ALOE++.](https://github.com/flexnets/aloe/wiki/ALOE-Project)

### Mail list
For any question, bug report or suggestion, please register to our mail list at
https://groups.google.com/group/flexnets


### News
 * Upcoming presentation/demo:
    * SDR-WinnComm-Europe, 11-13th June, Munich  
 * ALOE++ 0.6 has been released. New features:
    * Downlink Channels: PDSCH, PBCH, PCFICH and PDCCH 
    * TX and RX isolation
    * UDP source/sink modules
    * MATLAB/Octave models: PUSCH and PUCCH.
 * ALOE++ 0.4.1 has been released. New Features: 
    * Xenomai RTOS support to achieve lowest latencies.
    * Pipeline stage merge.
    * LTE PDSCH full TX and RX, 3 ms latency.  
    * LTE PDSCH Matlab model based on mex-files for verification.

### Requirements
To install ALOE++, the only requirement is the libconfig parsing library and cmake:
 * libconfig 1.4.8
 * cmake

The current ALOE++ release comes with an OFDM demo waveform and a small set of useful modues. These modules have more requirements: 
 * libfftw3 is used by the gen_dft module 
 * plplot 5.9.9 + output driver is used by the plp_sink module to display signals.

If you want to use the USRP, you should have the UHD driver and the Boost_thread library. 

To install all requirements in ubuntu, just type:

`sudo apt-get install libconfig-dev libfftw3-dev libplplot-dev plplot11-driver-xwin plplot11-driver-cairo`

You need libconfig-dev version 1.4.8 or higher. Older Ubuntu versions install older libconfig-dev versions. In this case (e.g., if 'libconfig8-dev' is installed), uninstall it (sudo apt-get remove libconfig8-dev) before downloading libconfig-dev from http://www.hyperrealm.com/libconfig/ and installing it manually: extract files, cd folder/, ./configure, make, sudo make install.

The last two packages are optional. PLplot can work with many different output drivers. The xwin driver works just fine and is fast. The driver used by plp_sink can be selected from the file modrep_ofdm/plp_sink/src/plp_sink.h (see the Documentation Section)

To be able to use Matlab for verification, type `export MATLAB_ROOT=/root/to/Matlab/folder` in a terminal.
  
### Download and Compile

Download aloe-0.6 from https://github.com/flexnets/aloe/archive/0.6.tar.gz and extract the contents:

```

wget https://github.com/flexnets/aloe/archive/0.6.tar.gz
tar xzvf 0.6.tar.gz
cd aloe-0.6
mkdir build
cd build
cmake ../

```

If you have Xenomai installed and want to use it, run the cmake command with `cmake ../ -DXENOMAI_ENABLE=1`

`make`

ALOE++ does not need to be installed to run. Installing is more convenient in order to create new components, since libraries and headers are installed in default locations. To do so, just type:

`sudo make install`


### Running the OFDM demo waveform
The OFDM test waveform is defined in file ofdm.app. It defines the DSP modules that build the waveform and their interconnections, among others. 

If you did not installed ALOE++, from the `aloe-0.6` directory, run:

`build/rtdal_lnx/runcf ./osld.app ./config`

The file `osld.app` defines the waveform graph. The file `config` stores the configuration of ALOE++. It allows to change the time slot length, the support for USRP devices and the location of the component libraries. 

To LOAD, INIT and RUN the waveform, just type:
 *  **l** and Enter, 
 *  **i** and Enter and then 
 *  **r** and Enter. 

You can also run in a step-by-step basis: pause the waveform typing "p" and then run a single time slot using "t". You can exit ALOE++ entering Ctrl+C in the shell window. 

### MATLAB/Octave Verification
ALOE++ now automatically creates a MEX-file for each module ([read here how](https://github.com/flexnets/aloe/wiki/Creating-a-DSP-Module)).

In the cloned directory you will find the file `ofdm_demo.m` which you can run from MATLAB or OCTAVE. You may need to edit the first line to adjust the path to where the MEX files where installed. This file calls the OFDM DSP modules one after another and plots the output signal. 

### Documentation 
For ALOE++ Developers:
 * RTDAL API: http://flexnets.github.com/aloe/rtdal/html/index.html
 * OESR API: http://flexnets.github.com/aloe/oesr/html/index.html
 * OESR Manager API: http://flexnets.github.com/aloe/oesr_man/html/index.html

To learn how to create new DSP Modules using the OESR SKELETON template:
 * [Creating a DSP Module](https://github.com/flexnets/aloe/wiki/Creating-a-DSP-Module)

Current waveforms documentation:
 * OSLD LTE TX/RX: http://flexnets.github.com/aloe/modrep_osld/html/index.html

### Related Projects
 * OSLD (https://sites.google.com/site/osldproject/)
 * FlexNets (http://flexnets.upc.edu)
 * GNU Radio (http://gnuradio.org)
 * OSSIE from Wireless@VT (http://ossie.wireless.vt.edu/)

[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/fbe47a2652453cdc1eb50219b38ab2f0 "githalytics.com")](http://githalytics.com/flexnets/aloe)
