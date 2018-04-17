@echo off

set EXEC=RTMStencil.exe
set SRCDIR=src\
set DESTDIR=release\
set EXTRA_INCLUDE=
set EXTRA_LIB=

rem if in Visual Studio environment
if /i "%PRODUCT_NAME%"=="" (
	set CC=cl
	set CC_FLAGS=/O2 /Ot /arch:AVX /GL
)
rem else if in Intel environment
if /i NOT "%PRODUCT_NAME%"=="" (
	set CC=icl
	set CC_FLAGS=/O2 /QxHOST /fp:fast /Qipo /MD
)

set LINK_FLAGS=/INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MANIFEST:NO

if /i "%1"=="clean" goto clean
if /i "%1"=="run" goto run

:options
if "%1"=="" goto compile

if /i "%1"=="vec-report" set CC_FLAGS=%CC_FLAGS% /Qvec-report1
if /i "%1"=="perf_num" set CC_FLAGS=%CC_FLAGS% -D PERF_NUM
shift
goto options

:compile
mkdir %DESTDIR% 2>nul
%CC% %CC_FLAGS% %EXTRA_INCLUDE% %SRCDIR%*.cpp /Fo%DESTDIR% /link %LINK_FLAGS% %EXTRA_LIB% /out:%DESTDIR%%EXEC%
goto eof

:run
%DESTDIR%%EXEC% %2
goto eof

:clean
echo removing files...
echo Y | rmdir /S %DESTDIR% >nul 2>nul
echo Y | del *.txt
:eof
