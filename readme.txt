Program: Enhanced Uptime Meter for OS/2, Version 1.1
====================================================

Author:  Tobias Ernst, e-mail: tobi@bland.fido.de, Fidonet: 2:2476/418.0
License: Released to the Public Domain. I do not claim any copyright for this.

This program realises an uptime meter for OS/2, i.E. it displays the number
of days, hours, minutes and seconds that have passed since the machine has
last been rebooted.

The method this program by default uses to determine the uptime is to
calculate the difference between the current date and the creation date of
the swap file.  In contrast to the method that most other OS/2 uptime
meters use (querying the OS/2 internal uptime timer), this has both
advantages and disadvantages:

  Advantages:
     - This method is able to correctly detect upimes greater than 25 and
       even greater than 50 days. In fact, it can handle uptimes up to
       about 60 years correctly.  (Programs querying the OS/2 uptime timer
       wrap to zero at 50 days, because the timer cannot hold values
       greater than 2^32 - 1  milliseconds).

  Disadvantages:
     - The program has to parse your CONFIG.SYS file in order to determine
       the location of your swap file. On some very queer installations,
       this may fail, and of course, it takes a little more time than
       the other method of querying the OS/2 internal uptime timer.

     - If you are using a notebook with hibernation feature or a PC like
       the IBM Aptiva  with RapidResume[tm], the time during which your
       computer is hibernated will (IMO incorrectly) also account for the
       uptime if you use this tool, while the OS/2 internal uptime timer
       is not increased while the system sleeps.

     - The value reported by this program is about 1 minute less than the
       value reported by the OS/2 internal uptime timer, because the swap
       file is only created about one minute after the OS/2 kernel has been
       booted.

If these disadvantages are a problem to you, you can also instruct the
program to query the OS/2 internal uptime timer instead of the swap file.

For running the program, you only need UPTIME.EXE.  Run it in an OS/2
windowed or fullscreen session.  Here are some usage examples:

   [C:\]uptime

       Running the program without parameters will make it query the uptime
       from the swyp file and report the result in an unix-like manner.

   [C:\]uptime -v

       This will give some more verbose information.

   [C:\]uptime -2

       This will force the program to query the OS/2 internal uptime timer
       instead of the swap file. This will yield incorrect values if your
       machine is running longer than about 50 days, but it is the only
       choice if you use a notebook with hibernation features.

   [C:\]uptime -h

       Prints a help screen.

The return code of this program (which can be found in the rc variable if
UPTIME.EXE is started from within a REXX script) is the uptime in days.
This could be used for a web server status display, for example.

The source code UPTIME.C can be compiled with about any OS/2 C compiler.  I
tested with EMX GCC, Borland C anc IBM CSet. The executable shipped is
compiled with Borland C.

Credits:

 - Thanks to the people from COMP.OS.OS2.BUGS for the idea of writing
   an uptime meter that queries the swap file. (I do not remeber who
   exactly first came up with it, unfortunately).

 - Thanks to IBM for providing us with an operating system that runs so
   stable that one can proudly use uptime meters. :-)

 - Thanks to John Moore and Joachim Benjamins for their input on the
   previous version of this tool.

If you have any comments, feel free to contact me at the address listed
above.

[EOF]
