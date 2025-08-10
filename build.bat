REM This auxiliary script first activates the VC developer environment variables,
REM then defers to present Makefile for remainder of compilation. 
REM This script is dependent on Build Tools for Visual Studio 2022 having been installed.
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64"
nmake