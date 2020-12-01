# SysLat_Software

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

I've defined 2 functions in the HTTP_Client_Async header file that are not associated with the class ("session") defined there. 
- What are these functions called? 
- Should I define them somewhere else?