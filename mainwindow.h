#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QStringList>
#include <vector>
#include <windows.h>// WinAPI — для работы с низкоуровневыми функциями Windows (процессы, токены и т.д.)
#include <psapi.h>// PSAPI — API для получения списка процессов, модулей и информации о них :contentReference[oaicite:0]{index=0}
#include <QScrollBar>//Для работы со скролл-барами в виджетах (таблицы, списки)
#include <QMessageBox>
//запрос php
#include <QNetworkAccessManager>//Для управления сетевыми запросами (отправка/получение)
#include <QNetworkRequest>//Для формирования сетевых запросов
#include <QNetworkReply>//Для получения ответа на сетевой запрос
#include <QJsonObject>//Для работы с JSON-объектами (формирование/чтение)
#include <QJsonDocument>//Для сериализации/десериализации JSON объектов
#include <QDateTime>//Для работы с датами и временем (например, для генерации rid)
//закрытие и трей
#include <QSystemTrayIcon>//Для работы с иконкой в системном трее (свёрнутое приложение)
#include <QMenu>//Для создания контекстного меню (например, при щелчке по иконке трея)
#include <QCloseEvent>//Для перехвата события закрытия окна

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    std::vector<std::pair<int, QString>> getProcessList();//получение списка процессов(PID, имя)
    void show_processes();//обновить таблицу

    QString getdll();//получить список DLL / модулей выбранного процесса
    QString XOR();//шифрование (XOR + base64) данных для отправки
    QString XOR_return();//дешифрование (обратное XOR + base64)
    void checkDublicate();//проверить дубликаты запущенных экземпляров (самого себя)
private slots:
    void on_Exit_triggered();//fILE->Exit
    void on_end_taskButton_clicked();//Завершить процесс
    void on_send_dataButton_clicked();
    void on_get_dataButton_clicked();
    void on_RestartButton_clicked();//кнопка Restart with Admin
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);//щелчок по иконке трея
    void onActionShow();//пункт меню «Show» трее (показать окно)
    void onActionExit();//пункт меню «Exit» трее (показать окно)
protected:
    void timerEvent(QTimerEvent *);//периодичность обновления
    void closeEvent(QCloseEvent *event) override;//событие закрытия окна
private:
    Ui::MainWindow *ui;
    bool admin =false;//запущено ли приложение с правами администратора
    int timerId;//Идентификатор таймера, возвращённый startTimer — нужен для управления таймером
    bool firsttimer=true;//первое срабатывание таймера — чтобы один раз выполнить проверку дубликатов
    static const int DELAY = 140;//скорость обновления
    QNetworkAccessManager *manager;//Указатель на объект для отправки HTTP-запросов / получения ответов
    QString data;//Строка данных, которые будут зашифрованы и отправлены / расшифрованы после получения
    const QString key = "My_Key";  // статичный Ключ для XOR-шифрования / дешифрования
    QSystemTrayIcon *trayIcon;//Иконка в системном трее
    QMenu *trayMenu;//Контекстное меню для иконки трея
    QAction *actionShow;//Действие «Show» в меню трея
    QAction *actionExit;// Действие «Exit» в меню трея
};
#endif // MAINWINDOW_H
