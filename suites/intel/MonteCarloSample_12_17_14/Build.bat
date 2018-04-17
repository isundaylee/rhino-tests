@echo off

set EXEC=MonteCarlo.exe
set SRCDIR=src\
set DESTDIR=release\

rem if in Visual Studio environment
if /i "%PRODUCT_NAME%"=="" (
	set CC=cl
	set CC_FLAGS=/O2 /arch:AVX /Oi /fp:precise /GL
)
rem else if in Intel environment
if /i NOT "%PRODUCT_NAME%"=="" (
	set CC=icl
	set CC_FLAGS=/O2 /QxHost /Oi /fp:source /Qfast-transcendentals /Qimf-precision=high /Qvec-report1
)

set LINK_FLAGS=/INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MANIFEST:NO

if /i "%1"=="clean" goto clean
if /i "%1"=="run" goto run

:options
if "%1"=="" goto compile

if /i "%1"=="MKL" set CC_FLAGS=%CC_FLAGS% /Qmkl -D IS_USING_MKL
if /i "%1"=="perf_num" set CC_FLAGS=%CC_FLAGS% -D PERF_NUM
shift
goto options

:compile
mkdir %DESTDIR% 2>nul
%CC% %CC_FLAGS% /Fo%DESTDIR% %SRCDIR%*.cpp -link %LINK_FLAGS% /out:%DESTDIR%%EXEC%
goto eof

:run
%DESTDIR%%EXEC% %2
goto eof

:clean
rmdir /Q /S %DESTDIR% 2>nul
echo removing files...
:eof
