:: This auxiliary script activates the VC developer environment variables.
:: The script then opens a Git Bash session to enable compilation.
::
:: Q: Why Git Bash?
:: A: It's a convenient way to work in Linux-style forward-slash paths ('/') inside build commands.
:: Otherwise, compiledb will break, as it is not able to correctly handle Windows-style backslashes ('\')
:: inside clang invocations. 
::
:: This script is dependent on Build Tools for Visual Studio 2026 and Git Bash having been installed.
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64"
)
:: Let clang and ASAN DLLs be available in PATH.
set "PATH=C:\Program Files\LLVM\lib\clang\23\lib\windows;C:\Program Files\LLVM\bin;%PATH%"
start "" "C:\Program Files\Git\bin\bash.exe"