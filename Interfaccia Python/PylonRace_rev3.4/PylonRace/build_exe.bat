@echo off
REM Crea PylonRace.exe (singolo file, senza console). Eseguire su Windows con Python 3.10+.
cd /d "%~dp0"
python -m venv .venv || goto err
call .venv\Scripts\activate.bat
python -m pip install --upgrade pip
pip install -r requirements.txt || goto err
pyinstaller --noconfirm --onefile --windowed --name PylonRace_rev3.4 --icon pylonrace\data\icon.ico ^
  --add-data "pylonrace\data;pylonrace\data" ^
  --hidden-import pyttsx3.drivers --hidden-import pyttsx3.drivers.sapi5 --hidden-import comtypes.client ^
  run.py || goto err
echo.
echo Fatto: dist\PylonRace_rev3.4.exe
pause
exit /b 0
:err
echo ERRORE durante la creazione.
pause
exit /b 1
