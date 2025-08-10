:: This auxiliary script first activates the VC developer environment variables,
:: if they have not already been defined.
::
:: Then the remainder of compilation is deferred to present Makefile.
:: This script is dependent on Build Tools for Visual Studio 2022 having been installed.
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64"
)
nmake