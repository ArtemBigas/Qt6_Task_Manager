#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timerId = startTimer(DELAY);//запускаем таймер, с скоростью DELAY
    //проверяем кто запустил программу(пользователь или админ)
    HANDLE hToken = NULL;//Объявляем дескриптор токена доступа, инициализируем NULL
    // Пробуем открыть токен текущего процесса, чтобы узнать, запущен ли с правами админа :contentReference[oaicite:1]{index=1}
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;// Структура, куда поместим информацию об elevation (привилегиях)
        DWORD dwSize = 0;//Размер возвращаемых данных
        //Получаем информацию о повышенных правах токена — Elevation
        if (GetTokenInformation(hToken, TokenElevation,&elevation, sizeof(elevation),&dwSize))
        {
            admin = (elevation.TokenIsElevated != 0);//# Если токен elevated — считаем, что у нас права администратора
        }
        CloseHandle(hToken);//Закрываем дескриптор токена (освобождаем ресурс)
    }
    if (QSystemTrayIcon::isSystemTrayAvailable()) {//Если на системе есть поддержка системного трея
        trayIcon = new QSystemTrayIcon(this);//Создаём иконку трея
        //Устанавливаем иконку
        trayIcon->setIcon(QIcon(QCoreApplication::applicationDirPath()+"/image/qt_icon.png"));
        // Меню в трее:
        trayMenu = new QMenu(this);
        actionShow = trayMenu->addAction("Show");//Добавляем пункт меню «Show»
        actionExit = trayMenu->addAction("Exit");//Добавляем пункт меню «Exit»
        connect(actionShow, &QAction::triggered, this, &MainWindow::onActionShow);//Связываем сигнал нажатия «Show» с соответствующим слотом
        connect(actionExit, &QAction::triggered, this, &MainWindow::onActionExit);//Связываем сигнал «Exit»
        connect(trayIcon, &QSystemTrayIcon::activated,this, &MainWindow::trayIconActivated);//Связываем сигнал активации (щелчок) иконки трея
        trayIcon->setContextMenu(trayMenu);//Устанавливаем  меню как контекстное для трея
        trayIcon->show();//Показываем иконку трея
    }}

MainWindow::~MainWindow()
{
    delete ui;
    delete trayIcon;//Удаляем объект иконки трея
}
void MainWindow::closeEvent(QCloseEvent *event) {//ловим событие закрытия окна
//Если есть иконка трея и она видима — вместо закрытия просто скрываем окно, без этого программа закроется только через убийство процесса
    if (trayIcon && trayIcon->isVisible()) {
        hide();//Скрываем текущее окно
        event->ignore();//Игнорируем событие закрытия
    } else {
        QMainWindow::closeEvent(event);//иначе выполняем стандартное поведение
    }
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {//нажали на иконку
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {//Если это щелчок или двойной щелчок
        showNormal();//Показываем окно (разворачиваем)
        activateWindow();//Даём окну фокус
    }
}

void MainWindow::onActionShow() {// пункт меню «Show»
    showNormal();//Показываем окно (разворачиваем)
    activateWindow();//Даём окну фокус
}

void MainWindow::onActionExit() {//пункт меню «Exit»
    trayIcon->hide();//Скрываем иконку трея
    qApp->quit();//Завершаем приложение
}


void MainWindow::on_RestartButton_clicked()//кнопка Restart with Admin
{
    if (admin) {//Если уже запущено с правами администратора
        //Показываем критическое сообщение
        QMessageBox::critical(nullptr,QObject::tr("Already elevated"),QObject::tr("The program already has administrator rights"));
    } else {
        wchar_t exePath[MAX_PATH];// Путь к исполняемому файлу
        //запрашиваем путь к текущему exe, заодно проверяем его доступность
        if (!GetModuleFileNameW(nullptr, exePath, MAX_PATH)) {//если не можем получить путь, выводим сообщение
            QMessageBox::critical(nullptr,QObject::tr("Error"),QObject::tr("Cannot determine application path"));
            return;
        }

        //Структура для запуска нового процесса через ShellExecuteEx
        SHELLEXECUTEINFOW sei{};
        sei.cbSize = sizeof(sei);//Указываем размер структуры
        sei.lpVerb = L"runas";//Указываем, что хотим запустить «с правами администратора»
        sei.lpFile = exePath;//Указываем файл — текущий exe
        sei.lpParameters = GetCommandLineW();//Передаём текущие параметры командной строки
        sei.nShow = SW_SHOWNORMAL;//Окно запускаем нормально

        if (!ShellExecuteExW(&sei)) {//Попытка запуска нового процесса+проверка доступности
            // Если пользователь отказался или произошла ошибка
            QMessageBox::warning(nullptr,
                                 QObject::tr("Elevation failed"),
                                 QObject::tr("Failed to restart with administrator rights"));
        } else {
            //если все запустилось - закрыть текущую программу
            qApp->exit();
        }
    }
}

void MainWindow::on_send_dataButton_clicked()
{

    int row = ui->process_tableWidget->currentRow();//индекс выбранной строки в таблице процессов
    if (row < 0) {//Если ничего не выбрано
        QMessageBox::warning(this, "Error", "No process selected");
        return;
    }
    QTableWidgetItem *itemPid = ui->process_tableWidget->item(row, 0);//ячейка с PID
    QTableWidgetItem *itemName = ui->process_tableWidget->item(row, 1);//ячейка с name
    data=getdll();//Получаем список DLL (модулей) выбранного процесса
    data=XOR();//Шифруем  полученные данные
    //Генерируем уникальный идентификатор rid на основе времени(кол-во милисекунд с 01.01.1970)
    QString rid = QString::number(QDateTime::currentMSecsSinceEpoch());
    QJsonObject obj;//JSON-объект для запроса
    //Сам запрос
    obj["cmd"] = 1;
    obj["rid"] = rid;
    obj["data"] = data;
    QJsonDocument doc(obj);//JSON-документ из объекта
    QByteArray postData = doc.toJson(QJsonDocument::Compact);//JSON в компактный формат для отправки
    QNetworkRequest request(QUrl("http://172.245.127.93/p/applicants.php"));//HTTP-запрос
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");//Устанавливаем заголовок — JSON
    manager = new QNetworkAccessManager(this);//QNetworkAccessManager для отправки запроса
    QNetworkReply *reply = manager->post(request, postData);//Отправляем POST-запрос с данными
    connect(reply, &QNetworkReply::finished, this, [this, reply, rid]() {//Подключаем лямбда-слот на завершение запроса
        if (reply->error() == QNetworkReply::NoError) {//Если ошибки нет
            QByteArray resp = reply->readAll();//Читаем ответ
            QMessageBox::information(this, "Request sent", "rid: " + rid);//сообщение об отправке+rid
        } else {//Если ошибка сети
            QMessageBox::warning(this, "Network Error", reply->errorString());
        }
        reply->deleteLater();//Освобождаем объект ответа
        manager->deleteLater();//Освобождаем менеджер
    });
}


void MainWindow::on_get_dataButton_clicked()
{//считываем требуемый rid
    QString rid = ui->ridEdit->text().trimmed();
    if (rid.isEmpty()) {//если строка пуста
        QMessageBox::warning(this, "Error", "Please enter rid");
        return;
    }
    //JSON-объект для запроса
    QJsonObject obj;
    //Сам запрос
    obj["cmd"] = 2;
    obj["rid"] = rid;
    QJsonDocument doc(obj);//JSON-документ из объекта
    QByteArray postData = doc.toJson(QJsonDocument::Compact);//JSON в компактный формат для отправки
    QNetworkRequest request(QUrl("http://172.245.127.93/p/applicants.php"));//HTTP-запрос
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");//Устанавливаем заголовок — JSON
    manager = new QNetworkAccessManager(this);//QNetworkAccessManager для отправки запроса
    QNetworkReply *reply = manager->post(request, postData);//Отправляем POST-запрос с данными
    connect(reply, &QNetworkReply::finished, this, [this, reply, rid]() {//Подключаем лямбда-слот на завершение запроса
        if (reply->error() == QNetworkReply::NoError) {//Если ошибки нет
            QByteArray resp = reply->readAll();//Читаем ответ
            QJsonDocument respDoc = QJsonDocument::fromJson(resp);//преобразуем JSON-текст в QJsonDocument
            if (!respDoc.isNull() && respDoc.isObject()) {//Если JSON и объект валиден
                QJsonObject respObj = respDoc.object();//JSON-объект для чтения ответа
                QString respRid = respObj.value("rid").toString();//Читаем rid из ответа
                data   = respObj.value("data").toString();//Читаем  данные
                data = XOR_return();//Дешифруем
                // rid совпадает (по спецификации)
                if (respRid == rid) {// проверка целостности
                    QMessageBox::information(this, "Response", data);//показываем данные
                } else {
                    QMessageBox::warning(this, "Error", "rid from server differs");//ошибка данных
                }
            } else {
                QMessageBox::warning(this, "Error", "Invalid JSON received");//ошибка запроса
            }
        } else {
            QMessageBox::warning(this, "Network Error", reply->errorString());//ошибка сети
        }
        reply->deleteLater();//Освобождаем объект ответа
        manager->deleteLater();//Освобождаем менеджер
    });
}

void MainWindow::timerEvent(QTimerEvent *e) {//цикл вывода процессов

    Q_UNUSED(e);
    int selectedRow = ui->process_tableWidget->currentRow();//текущая выбранная строка
    int vpos = ui->process_tableWidget->verticalScrollBar()->value();//положение скролла
    QModelIndex topIndex = ui->process_tableWidget->indexAt(QPoint(0, 0));//индекс строки на вершине списка
    int topRow = topIndex.isValid() ? topIndex.row() : 0;//Если индекс валиден — номер строки, иначе 0
    show_processes();//Обновляем список процессов (перезаполняем таблицу)
    ui->process_tableWidget->selectRow(selectedRow);//Снова выделяем строку, которая была выделена до обновления
    if (topRow >= 0 && topRow < ui->process_tableWidget->rowCount()) {//Если предыдущая верхняя строка всё ещё существует
        QTableWidgetItem *item = ui->process_tableWidget->item(topRow, 0);//Получаем элемент (PID) этой строки
        if (item) {
            ui->process_tableWidget->scrollToItem(item, QAbstractItemView::PositionAtTop);//Прокручиваем таблицу, чтобы эта строка была вверху
        }
    }
    ui->process_tableWidget->verticalScrollBar()->setValue(vpos);//Восстанавливаем прежнее положение скролла
    //проверка на дубль
    if(firsttimer==true){checkDublicate();firsttimer=false;}
    repaint();
}
void MainWindow::on_Exit_triggered(){qApp->exit();}

void MainWindow::on_end_taskButton_clicked()
{
    int row = ui->process_tableWidget->currentRow();// выбранная строка
    if (row < 0) {//Если ни одной строки не выбрано
        QMessageBox::warning(this, "Error", "No process selected");
        return;
    }

    QTableWidgetItem *pidItem = ui->process_tableWidget->item(row, 0);
    if (!pidItem) {//Если не удалось прочитать PID
        QMessageBox::warning(this, "Error", "Cannot read PID");
        return;
    }

    bool ok;
    DWORD pid = pidItem->text().toUInt(&ok);//текст PID в число
    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid PID");
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);//открыть процесс с правом TERMINTATE (завершение)
    if (!hProcess) {// Если не удалось — нет прав либо процесс защищён
        QMessageBox::warning(this, "Error", "Failed to open process");
        return;
    }

    BOOL killed = TerminateProcess(hProcess, 1);//Пытаемся завершить процесс (возвращает TRUE/FALSE)
    CloseHandle(hProcess);//Закрываем дескриптор процесса

    if (!killed) {//Если завершение не удалось
        QMessageBox::warning(this, "Error", "Failed to terminate process");
    } else {
        QMessageBox::information(this, "Success", "Process terminated");

        show_processes();
    }
}
std::vector<std::pair<int, QString>> MainWindow:: getProcessList() {
    std::vector<std::pair<int, QString>> result;//Вектор пар (PID, имя процесса)

    DWORD aProcesses[1024], cbNeeded, cProcesses;//Массив DWORD для PID
    //Вызываем WinAPI для получения PID всех процессов :contentReference[oaicite:2]{index=2}
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) //Если не удалось — возвращаем пустой список
        return result;

    cProcesses = cbNeeded / sizeof(DWORD);//сколько PID реально получено
    for (unsigned int i = 0; i < cProcesses; i++) {
        DWORD pid = aProcesses[i];
        if (pid == 0) continue;//Пропускаем PID 0 (системный Idle-процесс)
        //# Открываем процесс для чтения информации
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, pid);
        if (hProcess) {//Если удалось открыть процесс
            HMODULE hMod;//Буфер для модулей
            DWORD cbNeeded2;
            //# Получаем список загруженных модулей (DLL/EXE) этого процесса :contentReference[oaicite:3]{index=3}
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded2)) {
                TCHAR szName[MAX_PATH] = TEXT("<unknown>");//Буфер для имени модуля (exe)
                GetModuleBaseName(hProcess, hMod, szName, sizeof(szName)/sizeof(TCHAR));//Получаем базовое имя (имя exe)
                QString processName = QString::fromWCharArray(szName);//Преобразуем имя в QString
                result.emplace_back((int)pid, processName);//Добавляем (PID, имя) в результат
            }
            CloseHandle(hProcess);//Закрываем дескриптор процесса
        }
    }
    return result;//Возвращаем вектор процессов
}
void MainWindow::show_processes(){
    ui->process_tableWidget->clear();  // очистить старые данные
    ui->process_tableWidget->setColumnCount(2);//Устанавливаем 2 колонки: PID и Name
    ui->process_tableWidget->setHorizontalHeaderLabels(QStringList() << "PID" << "Name");//Заголовки колонок
    std::vector<std::pair<int, QString>> procs = getProcessList();//список процессов
    ui->process_tableWidget->setRowCount((int)procs.size());//число строк в таблице, равно числу процессов
    for (int i = 0; i < (int)procs.size(); i++) {//Заполняем строки таблицы
        auto &p = procs[i];
        ui->process_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(p.first)));//PID
        ui->process_tableWidget->setItem(i, 1, new QTableWidgetItem(p.second));//Имя процесса
    }
    ui->process_tableWidget->resizeColumnsToContents();//Подгоняем ширину колонок под содержимое
    ui->process_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//Выбор — целые строки
    ui->process_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//Только одна строка может быть выбрана
}


QString MainWindow::getdll(){
    int row = ui->process_tableWidget->currentRow();//Текущая выбранная строка
    QTableWidgetItem *itemPid = ui->process_tableWidget->item(row, 0);//PID
    if (!itemPid) return QString();//Если ошибка — вернуть пустую строку
    DWORD pid = itemPid->text().toUInt();//Преобразуем PID из текста
    if (pid == 0) return QString();//Если PID 0 — игнорируем
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, pid);
    if (!hProcess) return QString();//Если не удалось — возвращаем пустую строку
    HMODULE hMods[1024];//Буфер для модулей
    DWORD cbNeeded = 0;//Для количества возвращённых байт
    // Вызов PSAPI для получения списка модулей
    if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {//если не получаем
        CloseHandle(hProcess);//Закрываем дескриптор процесса
        return QString();//пустая строка
    }
    int count = cbNeeded / sizeof(HMODULE);//Считаем, сколько модулей получено
    QStringList dllList;//Список строк — для путей модулей
    for (int i = 0; i < count; i++) {//Перебираем все модули
        TCHAR szModName[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, hMods[i],szModName,sizeof(szModName)/sizeof(TCHAR))) {//полный путь модуля
            dllList << QString::fromWCharArray(szModName);//Добавляем путь в список
        }
    }
    CloseHandle(hProcess);//Закрываем дескриптор процесса
    return dllList.join("\n");//все пути через перевод строки
}

QString MainWindow::XOR(){
    QByteArray QBytedata = data.toUtf8();      // переводим QString в байты (UTF-8)
    QByteArray result;
    result.resize(QBytedata.size());// Выделяем буфер
    QByteArray keyBytes = key.toUtf8();//Ключ (key) тоже в QByteArray
    int keyLen = keyBytes.size();//Длина ключа
    for (int i = 0; i < QBytedata.size(); ++i) {//Для каждого байта данных
        result[i] = QBytedata[i] ^ keyBytes[i % keyLen];//XOR с соответствующим байтом ключа (циклично)
    }
    // преобразуем зашифрованные байты обратно в сначала base64, потом в QString
    return QString::fromUtf8(result.toBase64());
}

QString MainWindow::XOR_return(){
    //  Декодируем из Base64 → получаем QByteArray с зашифрованными байтами
    QByteArray enc = QByteArray::fromBase64(data.toUtf8());
    QByteArray keyBytes = key.toUtf8();
    int keyLen = keyBytes.size();
    //  Операция XOR обратно:
    QByteArray plain;
    plain.resize(enc.size());
    for (int i = 0; i < enc.size(); ++i) {
        plain[i] = enc[i] ^ keyBytes[i % keyLen];
    }
    //   возвращаем QString
    return QString::fromUtf8(plain);
}

void MainWindow::checkDublicate(){
    int duplicate=0;//количество процессов, идентичных текущему
    int rows = ui->process_tableWidget->rowCount();//количество строк в таблице
    for (int r = 0; r < rows; ++r) {//Перебираем все
        QTableWidgetItem *item = ui->process_tableWidget->item(r, 1);  // ищем по имени
        if (!item) continue;
        if (item->text() == "Qt6_Task_Manager.exe") {duplicate++;//находим наш процесс
            if(duplicate>1){//если наших процессов больше 1
            qApp->exit();
        };
        }
    }
}


