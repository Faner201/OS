@echo off
cd /d first_laboratory_work || (
    echo Не удалось перейти в директорию first_laboratory_work
    exit /b 1
)

cd 1 || (
    echo Не удалось перейти в директорию 1
    exit /b 1
)

for /f "delims=" %%i in ('git config --get remote.origin.url') do set repo_url=%%i

if not defined repo_url (
    echo Ошибка: не удалось получить URL репозитория.
    exit /b 1
)

set /p branch=Введите имя ветки для обновления (по умолчанию 'main'): 
if "%branch%"=="" set branch=main

git ls-remote --exit-code --heads %repo_url% %branch% >nul 2>&1
if %errorlevel%==0 (
    echo Обновление из ветки '%branch%'.
) else (
    echo Ветка '%branch%' не найдена. Используется 'main'.
    set branch=main
)

git pull %repo_url% %branch%
if %errorlevel% neq 0 (
    echo Не удалось обновить код из репозитория
    exit /b 1
)

call make -C first_laboratory_work build_and_clean
if %errorlevel% neq 0 (
    echo Не удалось собрать проект
    exit /b 1
)

echo Обновление, сборка и компиляция завершены.
