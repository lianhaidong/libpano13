PTpano12.dll

The purpose of this tool is to map all the functions called to pano12.dll.

The way it works is to rename the real pano12.dll to pano13.dll then put this
dll built as pano12.dll into the same directory. (windows\system32)

This tool will intercepts calls, displays info, and then calls pano13.dll with
the same parameters.  It then passes back the return values back to the 
original caller.

Using this technique it is possible to determine what calls are made, and 
what parameters passed, to and from pano12.dll.

The information gathered is logged into the file "c:\trace.txt"

By setting  DETAIL to nonzero more detailed information is recorded.


