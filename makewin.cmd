@echo off
set var=%1
set val=%2
echo var = %var%
echo val = %val%
IF /I "%val%" == "VALIDATE" (
    set val="-DUSE_VALIDATION_PARSER=on"
) ELSE (
    set val="-DUSE_VALIDATION_PARSER=off"
)

IF /I "%var%" == "WIN32" (
    echo calling makewin32.cmd %val%
    call makewin32.cmd %val%
) ELSE (
    IF /I "%var%" == "WIN32-x64" (
        echo calling makewin32x64.cmd %val%
        call makewin32x64.cmd %val%
    ) ELSE (
        IF /I "%var%" == "WIN32chk" (
            echo calling makewin32chk.cmd %val%
            call makewin32chk.cmd %val%
        ) ELSE (
            IF /I "%var%" == "WIN32-x64chk" (
                echo calling makewin32x64chk.cmd %val%
                call makewin32x64chk.cmd %val%
            ) ELSE (
                goto FAIL
            )
        )
    )
)
goto EXIT
:FAIL
echo specify one of [WIN32, WIN32-x64, WIN32chk, WIN32-x64chk] for 1st option
echo specify VALIDATE for 2nd option to disable XML schema validation.
EXIT /B 0
:Exit
echo done.
