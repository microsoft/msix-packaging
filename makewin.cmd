@echo off
set var=%1
echo var = %var%
IF /I "%var%" == "WIN32" (
    echo calling makewin32.cmd
    call makewin32.cmd
) ELSE (
    IF /I "%var%" == "WIN32-x64" (
        echo calling makewin32x64.cmd
        call makewin32x64.cmd
    ) ELSE (
        IF /I "%var%" == "WIN32chk" (
            echo calling makewin32chk.cmd
            call makewin32chk.cmd
        ) ELSE (
            IF /I "%var%" == "WIN32-x64chk" (
                echo calling makewin32x64chk.cmd
                call makewin32x64chk.cmd
            ) ELSE (
                goto FAIL
            )
        )
    )
)
goto EXIT
:FAIL
echo specify one of [WIN32, WIN32-x64, WIN32chk, WIN32-x64chk]
EXIT /B 0
:Exit
echo done.
