# SysLat_Software

Thank you for taking a look at SysLat. By the time you read this, this README will hopefully be organized into several sections such as:
* Goals for the project & TODO section
* FAQ
* How to build the program dynamically(the project builds statically linked by default).
* License

...but who knows. I may throw a bunch of things onto the website or into a wiki. If you're just looking for the latest version of SysLat and don't want to build and/or modify the program yourself, you can find it at https://SysLat.com/downloads.

SysLat's paired "firmware" can be found here: https://github.com/Skewjo/SysLat_Firmware

# Long-term Goals for this Project, Open Source, and a Message to the Community

I hope the measurements we take this with tool can lead to new advancements in the way both hardware and software are built, and I believe that with enough data this is possible through the use of machine learning and AI.
The only to get that data though, is by building a large community around a single standard tool for making these measurements. 
While the SysLat software may currently only run on Windows and may be poorly optimized, I'm hoping the open-source community can help me in porting it to additional operating systems and optimize it so that the impact on users is as minimal as possible.


#(REPO)TODO:
There was something I did to make this compile correctly at first - need to figure out what it was and document it.

Add vcpkg installation instructions and jsoncpp installation instructions(maybe make a wiki?)

Do I need a build server?


[To convert to static linkage](https://devblogs.microsoft.com/cppblog/vcpkg-updates-static-linking-is-now-available/):
  1. Use vcpkg to install the version of the static library you want access to. (in this case: .\vcpkg install jsoncpp:x86-windows-static)
  2. Create a new property page and add the following lines:
  ```
       <PropertyGroup Label="Globals">
        <VcpkgTriplet Condition="'$(Platform)'=='Win32'">x86-windows-static</VcpkgTriplet>
        <VcpkgTriplet Condition="'$(Platform)'=='x64'">x64-windows-static</VcpkgTriplet>
       </PropertyGroup>
  ```      
  3. Project->Properties->Advanced->Use of MFC->Use MFC in a Static Library
  4. Project->Properties->C/C++->Code Generation->Runtime Library->Multi-threaded (/MT)
  
To convert back to dynamic linkage:
  1. Delete the "PropertySheet.props" file mentioned above
  2. Project->Properties->Advanced->Use of MFC->Use MFC in a Shared DLL
  3. Project->Properties->C/C++->Code Generation->Runtime Library->Multi-threaded DLL (/MD)

When integrating the Boost/Beast library, I had to add a macro ```#undef max``` to file ```"file_stdio.ipp"```(I believe this is an external dependency, which means that every person that tries to build this thing from their workstation will have to make this change?? That can't be right...) because of some ancient problem or something. A quick Google/SO search will tell you all about it if you need to know more.

2/15/2021: This also happened (having to #undef min and #undef max) when I attempted to add [Howard Hinnant's Date library](https://howardhinnant.github.io/date/date.html).

I've defined 2 functions in the HTTP_Client_Async header file that are not associated with the class ("session") defined there. 
- What are these functions called? 
- Should I define them somewhere else?


# License
GNU General Public License v3.0 or later

See COPYING to see the full text.
