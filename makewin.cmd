@echo off
set build=%1
set val="-DUSE_VALIDATION_PARSER=off"
set zlib="-DUSE_SHARED_ZLIB=off"

:parseargs
IF /I "%~2" == "VALIDATE" (
    set val="-DUSE_VALIDATION_PARSER=on"
)
IF /I "%~2" == "SHARED_ZLIB" (
    set zlib="-DUSE_SHARED_ZLIB=on"
)
shift /2
IF not "%~2"=="" goto parseargs

echo val = %val%
echo zlib = %zlib%

IF /I "%build%" == "WIN32" (
    echo calling makewin32.cmd %val% %zlib%
    call makewin32.cmd %val% %zlib%
) ELSE (
    IF /I "%build%" == "WIN32-x64" (
        echo calling makewin32x64.cmd %val% %zlib%
        call makewin32x64.cmd %val% %zlib%
    ) ELSE (
        IF /I "%build%" == "WIN32chk" (
            echo calling makewin32chk.cmd %val% %zlib%
            call makewin32chk.cmd %val% %zlib%
        ) ELSE (
            IF /I "%build%" == "WIN32-x64chk" (
                echo calling makewin32x64chk.cmd %val% %zlib%
                call makewin32x64chk.cmd %val% %zlib%
            ) ELSE (
                goto FAIL
            )
        )
    )
)
goto EXIT
:FAIL
echo specify one of [WIN32, WIN32-x64, WIN32chk, WIN32-x64chk] for 1st option. Required.
echo other options: VALIDATE to enable XML schema validation. SHARED_ZLIB to don't statically link zlib. 
EXIT /B 0
:Exit
echo done.
