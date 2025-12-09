# Qt6_Task_Manager
1.Development environment: Visual Studio 2022
2.The process table consists of two columns: PID, Name
3.Buttons:
“End task” – terminates the selected process
“Send data” – sends a POST request using the sample format with data (see Technical_Specification.doc)
“Get data” – sends a POST request using the sample format to receive data (see Technical_Specification.doc) and displays it in a MessageBox
“Restart with Admin” – restarts the application with administrator privileges
4.All sent and received data is encrypted and decrypted using XOR encryption
5.The application checks the table for a duplicate of its own process, and if found, it closes automatically
6.The application starts by default with the privileges of its parent process. If it was launched with administrator rights, executing “Restart with Admin” is blocked
7.When resizing the window, widgets scale proportionally. The process table behaves the same as the one in the Windows 10 Task Manager. The minimum window size is 360×360
8.The application remembers the scroll position and the selected row, so no flickering occurs
9.When using File → Exit, the application closes; when clicking X, the main window hides and minimizes to the system tray. The tray icon is located in the image folder next to the executable
10.In the system tray, a context menu is available:
“Show” – restores the application from the tray
“Exit” – completely closes the application
11.The release version is processed through windeployqt6.exe, so it should run on Windows 10/11 without additional installations.  

1.Среда разработки: Visual Studio 2022  
2.Таблица процессов состоит из колонок  PID, Name  
3.Кнопки:  
-)«End task»-завершает процесс  
-)«Send data» - отправляет POST запрос по образцу с данными(см.Техническое_задание.doc)  
-) «Get data» - отправляет POST запрос по образцу для получения данных(см.Техническое_задание.doc) и выводи в MessageBox  
-)«Restart with Admin» - перезапускает программу с правами администратора  
4.Все отправляемые и получаемые данные шифруются и дешифруются через XOR-шифрование  
5.Программа проверяет таблицу на наличие дубля своего процесса и при нахождении автоматически закрывается  
6.Запуск по умолчанию с правами родительского процесса. Если программа была запушена с правами администратора, выполнение «Restart with Admin» блокируется  
7.При изменении размера окна виджеты растягиваются пропорционально. Таблица процессов ведет себя так же, как и аналогичная в Диспетчере задач в Windows 10.Минимальный размер окна 360х360  
8.Программа запоминает положение скролла и выделенной строки,всвязи с этим моргание отсутствует  
9.При использовании File->Exit программа закрывается,при нажатии X главное окно скрывается и сворачивается в трей.Картинка для трея находиться в папке image рядом с exe файлом   
10.В трее можно вызвать контекстное меню:    
-)"Show" - разворачивает приложение из трея  
-)«Exit» - осуществляет полный выход приложения  
11.Релизная версия пропущена через windeployqt6.exe, поэтому должна запускаться на Windows 10/11 без дополнительных установок.


