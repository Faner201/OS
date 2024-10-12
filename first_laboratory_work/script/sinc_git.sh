#!/bin/bash
repo_url=$(git config --get remote.origin.url)

if [ -z "$repo_url" ]; then
    echo "Ошибка: не удалось получить URL репозитория."
    exit 1
fi

read -p "Введите имя ветки для обновления (по умолчанию 'main'): " branch
branch=${branch:-main}

if git ls-remote --exit-code --heads "$repo_url" "$branch"; then
    echo "Обновление из ветки '$branch'."
else
    echo "Ветка '$branch' не найдена. Используется 'main'."
    branch="main"
fi

git pull "$repo_url" "$branch" || { echo "Не удалось обновить код из репозитория"; exit 1; }

make -C first_laboratory_work build_and_clean

if [ $? -ne 0 ]; then
    echo "Не удалось собрать проект"
    exit 1
fi

echo "Обновление, сборка и компиляция завершены."
