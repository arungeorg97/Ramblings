# Ramblings

-------------------------------------\_~~__(··)_~~_/-------------------------------------------

This repository contains some scripts, i wrote for stupid reasons. Saving it for a rainy day

- Pinj - Broswer injection:
  - Looks for running instances of msedge / iexplorer / chrome in that order and try to inject the shellcode.
  - If no running instance is found, it spawns a new edge.exe instance in suspended mode, write the shellcode and resumes execution.
  - Shellcode is passed as a header file.
  - Uses simple Win32 API calls.
  
