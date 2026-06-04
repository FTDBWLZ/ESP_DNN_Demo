@echo off
setlocal

set "IDF_PATH=E:\Espressif_V5.5.4"
set "IDF_TOOLS_PATH=E:\Espressif_Tools\Espressif"

set "IDF_GIT=E:\Espressif_Tools\Espressif\tools\idf-git\2.44.0\cmd"
set "IDF_PYTHON_ROOT=E:\Espressif_Tools\Espressif\tools\idf-python\3.11.2"
set "IDF_PYTHON_ENV=E:\Espressif_Tools\Espressif\python_env\idf5.5_py3.11_env\Scripts"
set "IDF_CMAKE=E:\Espressif_Tools\Espressif\tools\cmake\3.30.2\bin"
set "IDF_NINJA=E:\Espressif_Tools\Espressif\tools\ninja\1.12.1"
set "IDF_EXE=E:\Espressif_Tools\Espressif\tools\idf-exe\1.0.3"
set "IDF_CCACHE=E:\Espressif_Tools\Espressif\tools\ccache\4.10.2\ccache-4.10.2-windows-x86_64"
set "IDF_XTENSA=E:\Espressif_Tools\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin"
set "IDF_RISCV=E:\Espressif_Tools\Espressif\tools\riscv32-esp-elf\esp-14.2.0_20241119\riscv32-esp-elf\bin"
set "IDF_OPENOCD=E:\Espressif_Tools\Espressif\tools\openocd-esp32\v0.12.0-esp32-20241016\openocd-esp32\bin"

set "PATH=%IDF_GIT%;%IDF_PYTHON_ENV%;%IDF_PYTHON_ROOT%;%IDF_CMAKE%;%IDF_NINJA%;%IDF_EXE%;%IDF_CCACHE%;%IDF_XTENSA%;%IDF_RISCV%;%IDF_OPENOCD%;%PATH%"

call "%IDF_PATH%\export.bat"
if errorlevel 1 exit /b %errorlevel%

if "%~1"=="" (
    cmd /k
    exit /b 0
)

%*
