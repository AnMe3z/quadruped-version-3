# Мобилен робот с безжично управление
## _MECHKA v3_

[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger)

Целта на дипломната работа е да се създаде четирикрак робот с безжично управление.

Дипломант:     Андрей Методиев Ежков 12Г Випуск 2024

Ръководител: Владимир Иванов

В текущата репозитория е разделена на:

- hardware/ - четежи, CAD файлове и рендери
- software/ - сорс код на изработката

## hardware

Тук се намират всички чертежи по корпуса, CAD файлове, рендери и файловете за платката:

- body/
- leg/
- motor/
- schematics/

Както и 3Д рендер на цялостния робот

## software

Тук се намира сорс кода на проекта

- main/
- main.c
- control-app/
- app.c

main.c е софтуерът за микроконтролера. Инструкции за флашване:

```sh
cd software/main/build
chmod +x upload.sh
./upload.sh
```

app.py е GUI-то за контрол върху робота. Инструкции:

```sh
cd software/control-app/
source venv/bin/activate
python3 app.py
```

## License

Copyright 2023-2024
Андрей Ежков си запазва всички права
