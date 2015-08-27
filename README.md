# Disk-Optimizer
Write files optimally on disks.

Besides external HDDs and cloud storage, optical storage facilities (CDs, DVDs, etc.) are still used for data backup. It is often of interest how to place data on
them so that the minimum number of disks are required. Manual distribution for many files are cumbersome, therefore some kind of automation is welcome.

Currently the Best Fit Decreasing algorithm is used.

## Installation
The [master](https://github.com/CsatiZoltan/Disk-Optimizer/tree/master) branch contains the latest tested version so that should be downloaded. The executable is built with [Microsoft Visual Studio 2013](https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx) on a 64 bit Windows 7 machine, therefore the [x64 version](http://www.microsoft.com/en-us/download/details.aspx?id=40784) of the Visual Studio Runtime Library is needed. You can also compile the source file with other C compilers (like [tcc](http://bellard.org/tcc/)).

## Usage
Open a command prompt (cmd) and navigate where your executable is. Then type using the following syntax:
```
DO -option1 -option2 ... inputFileName
```
See the [documentation](https://github.com/CsatiZoltan/Disk-Optimizer/blob/1.2/doc/Manual.pdf) for detailed description.
