#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void displayBooks();
    int loggedInUserId = -1;
    QString loggedInUserRole;


private slots:
    void insertBook();
    void login();
    void createAccount();
    void borrowBook();
    void displayBorrowedBooks();
    void searchBooks();
    void deleteSelectedBook();
    void  deleteSelectedUser();
    void displayUsers();
    void on_pushButtonViewUsers_clicked();
    void returnSelectedBook();
    void editSelectedBook();
    void reviewBooks();
    void on_pushButtonViewBooks_clicked();
    void loadComboBoxes();
    void loadAuthors();




private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
