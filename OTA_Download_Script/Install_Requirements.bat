@echo off
REM Check if Python is installed
python --version >nul 2>&1

REM If Python is not found, the error level will be set to 1
if %errorlevel% neq 0 (
    echo Nao foi possivel encontrar uma instalação de python em sua maquina.
    echo Para utilizar o script de download de dados, realize a instalacao do Python
	echo e execute novamente este script. Link para download: https://www.python.org/downloads/
) else (
    echo Instalacao de Python encontrada! Prosseguindo com a instalacao de dependencias
    echo:
    echo ====================================================================================================
    echo ==================================== AUTOMACAO INSTALACAO DE PACOTES ===============================
    echo ====================================================================================================
    echo:
    
    python -m pip install pyserial openpyxl datetime rich

    echo:
    echo ====================================================================================================
    echo =========================== INSTALACAO FINALIZADA, VERIFIQUE RESULTADOS ACIMA ======================
    echo ====================================================================================================
    echo:
)

REM Pause to keep the window open
pause
