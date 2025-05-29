#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>
#include <QInputDialog>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{


    ui->setupUi(this);
    ui->groupBoxInsertBook->setVisible(false); // Hide Insert Book section by default

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("library");
    db.setUserName("root");
    db.setPassword("708090Bm@");
    db.setPort(3306);
    db.setConnectOptions("MYSQL_OPT_SSL_MODE=SSL_MODE_DISABLED");
    if (!db.open()) {
        qDebug() << "Database Error: " << db.lastError().text();
        qDebug()<<QSqlDatabase::drivers();
        qDebug() << QCoreApplication::libraryPaths();

        QMessageBox::critical(this, "Database Connection Error", db.lastError().text());
    } else {
        qDebug() << "Database connected successfully.";
    }
    // Hide new entry fields initially
    ui->lineEditNewPublisher->hide();
    ui->lineEditNewAuthorFirst->hide();
    ui->lineEditNewAuthorLast->hide();
    ui->lineEditNewCategory->hide();
    loadAuthors();


    // Show line edit if "Add new..." is selected in combo box
    connect(ui->comboBoxPublisher, &QComboBox::currentTextChanged, this, [=](const QString &text){
        ui->lineEditNewPublisher->setVisible(text == "Add new...");
    });
    loadComboBoxes();
   /* ui->comboBoxPublisher->addItem("Add new...");
    ui->comboBoxAuthor ->addItem("Add new...");
    ui->comboBoxCategory->addItem("Add new...");*/



    connect(ui->pushButtonInsert, &QPushButton::clicked, this, &MainWindow::insertBook);
    ui->comboBoxPublisher->setVisible("Add new...");
    connect(ui->comboBoxPublisher, &QComboBox::currentTextChanged, this, [=](const QString &text){
        ui->lineEditNewPublisher->setVisible(text == "Add new...");
    });
    connect(ui->comboBoxAuthor, &QComboBox::currentTextChanged, this, [=](const QString &text){
        bool isNew = (text == "Add new...");
        ui->lineEditNewAuthorFirst->setVisible(isNew);
        ui->lineEditNewAuthorLast->setVisible(isNew);
    });

    connect(ui->comboBoxCategory, &QComboBox::currentTextChanged, this, [=](const QString &text){
        ui->lineEditNewCategory->setVisible(text == "Add new...");
    });




    connect(ui->pushButtonLogin, &QPushButton::clicked, this, &MainWindow::login);
    connect(ui->pushButtonCreateAccount, &QPushButton::clicked, this, &MainWindow::createAccount);
    connect(ui->pushButtonBorrow, &QPushButton::clicked, this, &MainWindow::borrowBook);
    ui->pushButtonBorrow->setVisible(false);
    connect(ui->pushButtonViewBorrowed, &QPushButton::clicked, this, &MainWindow::displayBorrowedBooks);
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &MainWindow::searchBooks);
    connect(ui->pushButtonDeleteBook, &QPushButton::clicked, this, &MainWindow::deleteSelectedBook);
    connect(ui->pushButtonDeleteUser, &QPushButton::clicked, this, &MainWindow::deleteSelectedUser);
    connect(ui->pushButtonEditBook, &QPushButton::clicked, this, &MainWindow::editSelectedBook);
    connect(ui->pushButtonReturnBook, &QPushButton::clicked, this, &MainWindow::returnSelectedBook);
    ui->pushButtonViewUsers->setVisible(false);



    connect(ui->pushButtonReviewBooks, &QPushButton::clicked, this, &MainWindow::reviewBooks);


    ui->lineEditSearch->hide();
    ui->pushButtonSearch->hide();


    // Initially hidden


    ui->groupBoxCreateAccount->setVisible(false);
    connect(ui->pushButtonShowCreateAccount, &QPushButton::clicked, [this]() {
        ui->groupBoxLogin->setVisible(false);
        ui->groupBoxCreateAccount->setVisible(true);
        ui->pushButtonShowCreateAccount->setVisible(false);
    });




}

MainWindow::~MainWindow()
{
    delete ui;


}
void MainWindow::loadAuthors()
{
    QSqlQuery query("SELECT Author_ID, Author_first_name, last_name FROM authors");

    ui->comboBoxAuthor->clear();
    while (query.next()) {
        int id = query.value("Author_ID").toInt();
        QString fullName = query.value("Author_first_name").toString() + " " + query.value("last_name").toString();

        ui->comboBoxAuthor->addItem(fullName, id); // Store Author_ID as user data
    }

    ui->comboBoxAuthor->addItem("Add new..."); // Special option
}


void MainWindow::loadComboBoxes()
{
    QSqlQuery query;

    // Load Publishers
    ui->comboBoxPublisher->clear();
    query.exec("SELECT publisher_name FROM publishers");
    while (query.next()) {
        ui->comboBoxPublisher->addItem(query.value(0).toString());
    }
    ui->comboBoxPublisher->addItem("Add new...");

    // Load Authors (Full Name in one combo box)
    ui->comboBoxAuthor->clear();
    query.exec("SELECT Author_first_name, last_name FROM authors");
    while (query.next()) {
        QString fullName = query.value(0).toString() + " " + query.value(1).toString();
        ui->comboBoxAuthor->addItem(fullName);
    }
    ui->comboBoxAuthor->addItem("Add new...");

    // Load Categories
    ui->comboBoxCategory->clear();
    query.exec("SELECT Category_name FROM categories");
    while (query.next()) {
        ui->comboBoxCategory->addItem(query.value(0).toString());
    }
    ui->comboBoxCategory->addItem("Add new...");
}


void MainWindow::returnSelectedBook() {
    QItemSelectionModel *selectionModel = ui->tableViewBooks->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::warning(this, "No Selection", "Please select a book to return.");
        return;
    }

    QModelIndex index = selectionModel->currentIndex();
    int row = index.row();

    // Assuming Book_ID is in the 0th column
    int bookId = ui->tableViewBooks->model()->index(row, 0).data().toInt();

    QSqlQuery query;

    // Check if the book is borrowed by this user and is unavailable
    query.prepare(R"(
        SELECT ib.Due_date
        FROM books b
        JOIN issued_books ib ON b.Book_ID = ib.Book_ID
        WHERE b.Book_ID = :bookId AND b.Availability = 'Unavailable' AND ib.user_id = :userId
    )");
    query.bindValue(":bookId", bookId);
    query.bindValue(":userId", loggedInUserId);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Return Error", query.lastError().text());
        qDebug()<<query.lastError().text();
        return;
    }

    QDate dueDate = query.value("Due_date").toDate();
    QDate returnDate = QDate::currentDate();
    int overdueDays = returnDate.daysTo(dueDate) < 0 ? dueDate.daysTo(returnDate) : 0; // positive overdue days or 0
    int fine = overdueDays * 5;

    // Update book availability to 'available'
    query.prepare("UPDATE books SET Availability = 'available' WHERE Book_ID = :bookId");
    query.bindValue(":bookId", bookId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to update book availability: " + query.lastError().text());
        return;
    }

    // Delete from issued_books
    query.prepare("DELETE FROM issued_books WHERE Book_ID = :bookId AND user_id = :userId");
    query.bindValue(":bookId", bookId);
    query.bindValue(":userId", loggedInUserId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to remove issued book record: " + query.lastError().text());
        return;
    }

    QString msg = "Book returned successfully.";
    if (fine > 0) {
        msg += QString("\nOverdue fine: %1").arg(fine);
    }
    QMessageBox::information(this, "Return Success", msg);

    displayBooks(); // Refresh book list or UI
}




void MainWindow::reviewBooks() {
    on_pushButtonViewBooks_clicked();
}

void MainWindow::editSelectedBook() {
    QItemSelectionModel *selectionModel = ui->tableViewBooks->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::warning(this, "No Selection", "Please select a book to edit.");
        return;
    }

    QModelIndex index = selectionModel->currentIndex();
    int row = index.row();
    QString oldBookName = index.sibling(row, 1).data().toString();

    // Check availability
    QSqlQuery availabilityQuery;
    availabilityQuery.prepare("SELECT Availability FROM books WHERE Book_name = :name");
    availabilityQuery.bindValue(":name", oldBookName);

    if (!availabilityQuery.exec() || !availabilityQuery.next()) {
        QMessageBox::critical(this, "Error",   availabilityQuery.lastError().text());
        qDebug() << "Selected Book ID:" << oldBookName;

        return;
    }
    QString availability = availabilityQuery.value(0).toString();
    if (availability.toLower() != "available") {
        QMessageBox::warning(this, "Cannot Edit", "This book is currently borrowed and cannot be edited.");
        return;
    }

    // Fetch current publisher, author first/last names, and category
    QSqlQuery infoQuery;
    infoQuery.prepare(R"(
        SELECT p.publisher_name, a.Author_first_name, a.last_name, c.Category_name
        FROM books b
        LEFT JOIN publishers p ON b.Publisher_id = p.publisher_id
        LEFT JOIN authors a ON b.Author_ID = a.Author_ID
        LEFT JOIN categories c ON b.Category_ID = c.Category_ID
        WHERE b.Book_name = :name
    )");
    infoQuery.bindValue(":name", oldBookName);
    if (!infoQuery.exec() || !infoQuery.next()) {
        QMessageBox::critical(this, "Error", "Failed to get current book details: " + infoQuery.lastError().text());
        return;
    }
    QString currentPublisher = infoQuery.value(0).toString();
    QString currentAuthorFirst = infoQuery.value(1).toString();
    QString currentAuthorLast = infoQuery.value(2).toString();
    QString currentCategory = infoQuery.value(3).toString();

    // Prompt user for new details with defaults
    QString newBookName = QInputDialog::getText(this, "Edit Book", "Enter new book name:", QLineEdit::Normal, oldBookName).trimmed();
    if (newBookName.isEmpty()) {
        newBookName = oldBookName;
    }

    QString newPublisherName = QInputDialog::getText(this, "Edit Book", "Enter publisher name:", QLineEdit::Normal, currentPublisher).trimmed();
    if (newPublisherName.isEmpty()) {
       newPublisherName = currentPublisher;
    }

    QString newAuthorFirst = QInputDialog::getText(this, "Edit Book", "Enter author first name:", QLineEdit::Normal, currentAuthorFirst).trimmed();
    if (newAuthorFirst.isEmpty()) {
       newAuthorFirst = currentAuthorFirst;
    }

    QString newAuthorLast = QInputDialog::getText(this, "Edit Book", "Enter author last name:", QLineEdit::Normal, currentAuthorLast).trimmed();
    if (newAuthorLast.isEmpty()) {
          newAuthorLast = currentAuthorLast;
    }

    QString newCategoryName = QInputDialog::getText(this, "Edit Book", "Enter category name:", QLineEdit::Normal, currentCategory).trimmed();
    if (newCategoryName.isEmpty()) {
       newCategoryName = currentCategory;
    }

    QSqlQuery query;

    // Helper lambda to get or insert and get id
    auto getOrInsertId = [&](const QString &table, const QString &idField, const QString &fieldName, const QString &value) -> QVariant {
        query.prepare(QString("SELECT %1 FROM %2 WHERE %3 = :value").arg(idField, table, fieldName));
        query.bindValue(":value", value);
        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "SELECT failed on " + table + ": " + query.lastError().text());
            return QVariant();
        }
        if (query.next()) return query.value(0);

        // Insert if not found
        query.prepare(QString("INSERT INTO %1 (%2) VALUES (:value)").arg(table, fieldName));
        query.bindValue(":value", value);
        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "INSERT failed on " + table + ": " + query.lastError().text());
            return QVariant();
        }
        return query.lastInsertId();
    };



    // Get or insert publisher
    QVariant publisherId = getOrInsertId("publishers", "publisher_id", "publisher_name", newPublisherName);
    if (!publisherId.isValid()) {
        QMessageBox::critical(this, "Error", "Failed to get or add publisher.");
        return;
    }

    // Get or insert author
    query.prepare("SELECT Author_ID FROM authors WHERE Author_first_name = :first AND last_name = :last");
    query.bindValue(":first", newAuthorFirst);
    query.bindValue(":last", newAuthorLast);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to query authors: " + query.lastError().text());
        return;
    }
    QVariant authorId;
    if (query.next()) {
        authorId = query.value(0);
    } else {
        // Insert new author
        query.prepare("INSERT INTO authors (Author_first_name, last_name) VALUES (:first, :last)");
        query.bindValue(":first", newAuthorFirst);
        query.bindValue(":last", newAuthorLast);
        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to add new author: " + query.lastError().text());
            return;
        }
        authorId = query.lastInsertId();
    }

    // Get or insert category
   QVariant categoryId  = getOrInsertId("categories", "Category_ID", "Category_name", newCategoryName);
    if (!categoryId.isValid()) {
        QMessageBox::critical(this, "Error", "Failed to get or add category.");
        return;
    }

    // Update the book
    query.prepare(R"(
        UPDATE books SET
            Book_name = :newName,
            Publisher_id = :publisherId,
            Author_ID = :authorId,
            Category_ID = :categoryId
        WHERE Book_name = :oldName
    )");
    query.bindValue(":newName", newBookName);
    query.bindValue(":publisherId", publisherId);
    query.bindValue(":authorId", authorId);
    query.bindValue(":categoryId", categoryId);
    query.bindValue(":oldName", oldBookName);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to update book: " + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Success", "Book details updated.");
    displayBooks();
}





void MainWindow::on_pushButtonViewUsers_clicked()
{
    displayUsers(); // Populate the table with users

    // Show users table and delete button
    ui->tableViewUsers->setVisible(true);
    if(loggedInUserId==3){
         ui->pushButtonDeleteUser->setVisible(true);
    }else{ ui->pushButtonDeleteUser->setVisible(false);}



    ui->pushButtonReviewBooks->setVisible(true);

    // Hide books table and the view users button
    ui->tableViewBooks->setVisible(false);
    ui->pushButtonViewUsers->setVisible(false);
    ui->pushButtonSearch->setVisible(false);
    ui->lineEditSearch->setVisible(false);
    ui->pushButtonViewBorrowed->setVisible(false);
    ui->pushButtonDeleteBook->setVisible(false);
    ui->pushButtonEditBook->setVisible(false);


}
void MainWindow::on_pushButtonViewBooks_clicked()
{


    displayBooks();
    if (loggedInUserRole == "Librarian") {
        ui->pushButtonViewUsers->setVisible(true);
    }

    // Show books table and related buttons
    ui->tableViewBooks->setVisible(true);
    ui->pushButtonEditBook->setVisible(true);
    ui->pushButtonDeleteBook->setVisible(true);

    // Hide users table and view books button
    ui->tableViewUsers->setVisible(false);
    ui->pushButtonReviewBooks->setVisible(false);

    // Optional: Hide unrelated buttons (like delete user)
    ui->pushButtonDeleteUser->setVisible(false);
    ui->pushButtonEditUser->setVisible(false);
    ui->pushButtonSearch->setVisible(true);
    ui->lineEditSearch->setVisible(true);
    ui->pushButtonViewBorrowed->setVisible(true);
    ui->pushButtonDeleteBook->setVisible(true);
    ui->pushButtonEditBook->setVisible(true);


    // ui->pushButtonReviewBooks->setVisible(false);
}

void MainWindow::displayUsers()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);

    model->setQuery(R"(
        SELECT user_id AS 'ID',
               first_name AS 'First Name',
               last_name AS 'Last Name',
               mobile_number AS 'Mobile',
               role AS 'Role',
               username AS 'Username'
        FROM users
    )");

    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", model->lastError().text());
        return;
    }

    ui->tableViewUsers->setModel(model); // ✅ CORRECT
    // You can use a different tableView if available
    ui->tableViewUsers->resizeColumnsToContents();
    ui->pushButtonViewBorrowed->setVisible(true);
    ui->pushButtonDeleteBook->setVisible(true);

}

void MainWindow::deleteSelectedUser() {
    QItemSelectionModel *selectionModel = ui->tableViewUsers->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::warning(this, "No Selection", "Please select a user to delete.");
        return;
    }

    QModelIndex index = selectionModel->currentIndex();
    int row = index.row();

    QVariant userId = ui->tableViewUsers->model()->index(row, 0).data(); // user_id should be at column 0

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete this user?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM users WHERE user_id = :userId");
        query.bindValue(":userId", userId);
        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete user: " + query.lastError().text());
        } else {
            QMessageBox::information(this, "Success", "User deleted successfully.");
            displayUsers(); // You need to define this if not already
        }
    }
}

void MainWindow::deleteSelectedBook() {
    QItemSelectionModel *selectionModel = ui->tableViewBooks->selectionModel();
    if (!selectionModel->hasSelection()) {
        QMessageBox::warning(this, "No Selection", "Please select a book to delete.");
        return;
    }

    QModelIndex index = selectionModel->currentIndex();
    int row = index.row();

    // Find the Book_ID from the correct column (assume it's in column 0)
    // You must ensure that Book_ID is in the model even if hidden in the view
    QVariant bookId = ui->tableViewBooks->model()->index(row, 0).data();
    qDebug() << "Selected Book_ID:" << bookId;

    if (!bookId.isValid()) {
        QMessageBox::critical(this, "Error", "Failed to retrieve the Book_ID from the selected row.");
        return;
    }

    // Check availability from the database
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT Availability FROM books WHERE Book_ID = :bookId");
    checkQuery.bindValue(":bookId", bookId);
    if (!checkQuery.exec()) {
        QMessageBox::critical(this, "Error", "Failed to execute availability check: " + checkQuery.lastError().text());
        return;
    }
    if (!checkQuery.next()) {
        QMessageBox::critical(this, "Error", "No book found with the given ID.");
        return;
    }

    QString availability = checkQuery.value(0).toString();
    qDebug() << "Book availability:" << availability;

    if (availability.toLower() != "available") {
        QMessageBox::warning(this, "Cannot Delete", "This book is currently borrowed and cannot be deleted.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete this book?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM books WHERE Book_ID = :bookId");
        deleteQuery.bindValue(":bookId", bookId);
        if (!deleteQuery.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete the book: " + deleteQuery.lastError().text());
        } else {
            QMessageBox::information(this, "Success", "Book deleted successfully.");
            displayBooks(); // Refresh the book list
        }
    }
}

void MainWindow::searchBooks() {
    QString searchText = ui->lineEditSearch->text().trimmed();

    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Search", "Please enter a book name to search.");
        return;
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT
            books.Book_ID,
            books.Book_name,
            books.Availability,
            publishers.publisher_name,
            CONCAT(authors.Author_first_name, ' ', authors.last_name) AS author_full_name,
            categories.Category_name
        FROM books
        LEFT JOIN publishers ON books.Publisher_id = publishers.publisher_id
        LEFT JOIN authors ON books.Author_ID = authors.Author_ID
        LEFT JOIN categories ON books.Category_ID = categories.Category_ID
        WHERE books.Book_name LIKE :name
    )");
    query.bindValue(":name", "%" + searchText + "%");

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Search query failed: " + query.lastError().text());
        return;
    }

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(std::move(query));

    // Set headers for the displayed columns
    model->setHeaderData(0, Qt::Horizontal, "Book ID");
    model->setHeaderData(1, Qt::Horizontal, "Book Name");
    model->setHeaderData(2, Qt::Horizontal, "Availability");
    model->setHeaderData(3, Qt::Horizontal, "Publisher");
    model->setHeaderData(4, Qt::Horizontal, "Author");
    model->setHeaderData(5, Qt::Horizontal, "Category");

    if (model->rowCount() == 0) {
        QMessageBox::information(this, "No Results", "No books found with that name.");
        ui->tableViewBooks->setModel(nullptr);
        return;
    }

    ui->tableViewBooks->setModel(model);
    ui->tableViewBooks->setColumnHidden(0, true);  // Hide Book_ID column
    ui->tableViewBooks->resizeColumnsToContents();
}


void MainWindow::displayBorrowedBooks()
{
    QSqlQueryModel *model = new QSqlQueryModel;

    model->setQuery(R"(
        SELECT
            Issued_Books.Issue_ID,
            Issued_Books.Issue_date,
            Issued_Books.Due_date,
            Books.Book_name,
            users.first_name,
            users.last_name
        FROM
            Issued_Books
        JOIN
            Books ON Issued_Books.Book_ID = Books.Book_ID
        JOIN
            users ON Issued_Books.user_id = users.user_id
    )");

    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Error", model->lastError().text());
        delete model;
        return;
    }

    ui->tableViewBooks->setModel(model);
    ui->pushButtonReviewBooks->setVisible(true);
}

void MainWindow::borrowBook()
{
    QItemSelectionModel *selection = ui->tableViewBooks->selectionModel();
    if (!selection->hasSelection()) {
        QMessageBox::warning(this, "No Selection", "Please select a book to borrow.");
        return;
    }

    int row = selection->currentIndex().row();
    QAbstractItemModel *model = ui->tableViewBooks->model();

    QString bookName = model->data(model->index(row, 0)).toString(); // Assuming Book Name is column 0
    QString availability = model->data(model->index(row, 1)).toString(); // Availability is column 1

    if (availability.toLower() != "available") {
        QMessageBox::warning(this, "Unavailable", "This book is not available.");
        return;
    }

    // Retrieve the Book_ID from the database
    QSqlQuery query;
    query.prepare("SELECT Book_ID FROM books WHERE Book_name = :name");
    query.bindValue(":name", bookName);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Error", "Failed to get Book ID.");
        return;
    }

    int bookId = query.value(0).toInt();

    // Insert into Issued_Books
    QSqlQuery issueQuery;
    issueQuery.prepare(R"(
        INSERT INTO Issued_Books (Issue_date, Due_date, Book_ID, user_id)
        VALUES (CURDATE(), DATE_ADD(CURDATE(), INTERVAL 14 DAY), :bookId, :userId);
    )");
    issueQuery.bindValue(":bookId", bookId);
    issueQuery.bindValue(":userId", loggedInUserId);
    if (!issueQuery.exec()) {
        QMessageBox::critical(this, "Error", "Failed to borrow book: " + issueQuery.lastError().text());
        return;
    }
    QSqlQuery dueQuery;
    dueQuery.prepare(R"(
    SELECT Due_date FROM Issued_Books
    WHERE user_id = :userId AND Book_ID = :bookId
    ORDER BY Issue_ID DESC LIMIT 1
)");
    dueQuery.bindValue(":userId", loggedInUserId);
    dueQuery.bindValue(":bookId", bookId);

    QString dueDateStr = "unknown";
    if (dueQuery.exec() && dueQuery.next()) {
        dueDateStr = dueQuery.value(0).toDate().toString("yyyy-MM-dd");
    }

    // Update book availability
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE books SET Availability = 'Unavailable' WHERE Book_ID = :id");
    updateQuery.bindValue(":id", bookId);
    updateQuery.exec();

    QMessageBox::information(this, "Success", "You borrowed the book successfully.\nPlease return the book on or before the due date: " + dueDateStr);
    displayBooks(); // Refresh the book list
}

void MainWindow::displayBooks()
{
    // Model to hold the query result
    QSqlQueryModel *model = new QSqlQueryModel(this);

    // Set the query to select relevant book fields (adjust fields as needed)
    model->setQuery(R"(
        SELECT
 books.Book_ID,  -- add this column first
    books.Book_name AS 'Book Name',
    books.Availability,
    publishers.publisher_name AS 'Publisher',
    CONCAT(authors.Author_first_name, ' ', authors.last_name) AS 'Author',
    categories.Category_name AS 'Category'
FROM books
LEFT JOIN publishers ON books.Publisher_id = publishers.publisher_id
LEFT JOIN authors ON books.Author_ID = authors.Author_ID
LEFT JOIN categories ON books.Category_ID = categories.Category_ID
    )");

    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", model->lastError().text());
        return;
    }

    // Set model to the table view
    ui->tableViewBooks->setModel(model);
    ui->tableViewBooks->setModel(model);
    // Hide the Book_ID column, so it won't show but still accessible in model
    ui->tableViewBooks->setColumnHidden(0, true);


    // Optional: Adjust columns to fit content
    ui->tableViewBooks->resizeColumnsToContents();
    ui->tableViewBooks->setVisible(true);
    ui->tableViewUsers->setVisible(false);

    ui->pushButtonDeleteUser->setVisible(false);


}

void MainWindow::login()
{
    QString username = ui->lineEditLoginUsername->text().trimmed();
    QString password = ui->lineEditLoginPassword->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both username and password.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT user_id, Role FROM users WHERE username = :username AND Password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (query.exec() && query.next()) {
        QString role = query.value(1).toString();
        loggedInUserRole=role;
        loggedInUserId = query.value(0).toInt();
        QMessageBox::information(this, "Login Success", "Welcome, " + role);
        // After validating user login
        ui->lineEditSearch->show();
        ui->pushButtonSearch->show();


        ui->groupBoxLogin->setVisible(false);
        ui->groupBoxCreateAccount->setVisible(false);
        ui->pushButtonShowCreateAccount->hide();
        // Show/hide features based on role here
        if (role == "Librarian") {
            ui->groupBoxInsertBook->setVisible(true);
            on_pushButtonViewBooks_clicked();

        } else {
            ui->groupBoxInsertBook->setVisible(false);
            ui->pushButtonViewUsers->setVisible(false);

            ui->pushButtonReturnBook->setVisible(true);
            displayBooks();
            ui->pushButtonBorrow->setVisible(true);
        }
    } else {
        QMessageBox::warning(this, "Login Failed", query.lastError().text());
    }
}

void MainWindow::createAccount()
{
    QString firstName = ui->lineEditFirstName->text().trimmed();
    QString lastName = ui->lineEditLastName->text().trimmed();
    QString mobile = ui->lineEditMobileNumber->text().trimmed();
    QString role = ui->comboBoxRole->currentText();
    QString username = ui->lineEditUsername->text().trimmed();
    QString password = ui->lineEditPassword->text().trimmed();

    if (firstName.isEmpty() || lastName.isEmpty() || mobile.isEmpty() || username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO users (first_name, last_name, Mobile_Number, Role, username, password)
        VALUES (:firstName, :lastName, :mobile, :role, :username, :password)
    )");
    query.bindValue(":firstName", firstName);
    query.bindValue(":lastName", lastName);
    query.bindValue(":mobile", mobile);
    query.bindValue(":role", role);
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        QString errMsg = query.lastError().text().toLower();

        if (errMsg.contains("duplicate entry")) {
            QMessageBox::warning(this, "Error", "Username already exists. Please choose a different username.");
        } else {
            QMessageBox::critical(this, "Database Error", errMsg);
        }
        return;
    }

    QMessageBox::information(this, "Success", "Account created successfully!");

    // Automatic login after account creation
    QSqlQuery loginQuery;
    loginQuery.prepare("SELECT user_id, Role FROM users WHERE username = :username AND Password = :password");
    loginQuery.bindValue(":username", username);
    loginQuery.bindValue(":password", password);

    if (loginQuery.exec() && loginQuery.next()) {
        loggedInUserId = query.value(0).toInt();
        QString role = loginQuery.value(1).toString();
        loggedInUserRole=role;
        // After validating user login
        ui->lineEditSearch->show();
        ui->pushButtonSearch->show();


        // Hide login and create account UI
        ui->groupBoxLogin->setVisible(false);
        ui->groupBoxCreateAccount->setVisible(false);

        // Show Insert Book UI only if user is librarian
        if (role == "Librarian") {
            ui->groupBoxInsertBook->setVisible(true);
            ui->pushButtonViewBorrowed->setVisible(true);
            ui->pushButtonDeleteBook->setVisible(true);
            ui->pushButtonDeleteUser->setVisible(true);
            displayBooks();
        } else {
            ui->groupBoxInsertBook->setVisible(false);
            ui->pushButtonBorrow->setVisible(true);
            displayBooks();
        }
    } else {
        QMessageBox::warning(this, "Login Failed", "Automatic login failed after account creation.");
    }
}



void MainWindow::insertBook()
{
    QString bookName = ui->lineEditBookName->text().trimmed();
    QString availability = ui->lineEditAvailability_2->currentText().trimmed();

    // Check required fields
    if (bookName.isEmpty() || availability.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all required fields.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery query(db);
    QVariant publisherId, authorId, categoryId;

    // Handle Publisher
    QString selectedPublisher = ui->comboBoxPublisher->currentText();
    if (selectedPublisher == "Add new...") {
        QString newPublisher = ui->lineEditNewPublisher->text().trimmed();
        if (newPublisher.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please enter new publisher name.");
            return;
        }
        query.prepare("INSERT INTO publishers (publisher_name) VALUES (:name)");
        query.bindValue(":name", newPublisher);
        if (!query.exec()) {
            QMessageBox::critical(this, "Insert Error", "Failed to add new publisher:\n" + query.lastError().text());
            return;
        }
        publisherId = query.lastInsertId();
    } else {
        query.prepare("SELECT publisher_id FROM publishers WHERE publisher_name = :name");
        query.bindValue(":name", selectedPublisher);
        query.exec();
        if (query.next()) publisherId = query.value(0);
    }

    // Handle Author
    // Handle Author (Using Full Name)
    QString selectedAuthor = ui->comboBoxAuthor->currentText();
    if (selectedAuthor == "Add new...") {
        QString firstName = ui->lineEditNewAuthorFirst->text().trimmed();
        QString lastName = ui->lineEditNewAuthorLast->text().trimmed();

        if (firstName.isEmpty() || lastName.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please enter both first and last name for the author.");
            return;
        }


        query.prepare("INSERT INTO authors (Author_first_name, last_name) VALUES (:first, :last)");
        query.bindValue(":first", firstName);
        query.bindValue(":last", lastName);
        if (!query.exec()) {
            QMessageBox::critical(this, "Insert Error", "Failed to add new author:\n" + query.lastError().text());
            return;
        }
        authorId = query.lastInsertId();
    } else {
        QStringList nameParts = selectedAuthor.split(" ", Qt::SkipEmptyParts);
        if (nameParts.size() < 2) {
            QMessageBox::warning(this, "Input Error", "Selected author name format is invalid.");
            return;
        }
        QString firstName = nameParts.first();
        QString lastName = nameParts.mid(1).join(" ");

        query.prepare("SELECT Author_ID FROM authors WHERE Author_first_name = :first AND last_name = :last");
        query.bindValue(":first", firstName);
        query.bindValue(":last", lastName);
        query.exec();
        if (query.next()) authorId = query.value(0);
    }


    // Handle Category
    QString selectedCategory = ui->comboBoxCategory->currentText();
    if (selectedCategory == "Add new...") {
        QString newCategory = ui->lineEditNewCategory->text().trimmed();
        if (newCategory.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please enter new category name.");
            return;
        }
        query.prepare("INSERT INTO Categories (Category_name) VALUES (:name)");
        query.bindValue(":name", newCategory);
        if (!query.exec()) {
            QMessageBox::critical(this, "Insert Error", "Failed to add new category:\n" + query.lastError().text());
            return;
        }
        categoryId = query.lastInsertId();
    } else {
        query.prepare("SELECT Category_ID FROM Categories WHERE Category_name = :name");
        query.bindValue(":name", selectedCategory);
        query.exec();
        if (query.next()) categoryId = query.value(0);
    }

    // Final insert into books
    query.prepare(R"(
        INSERT INTO books (Book_name, Availability, Publisher_id, Author_ID, Category_ID)
        VALUES (:bookName, :availability, :publisherId, :authorId, :categoryId)
    )");
    query.bindValue(":bookName", bookName);
    query.bindValue(":availability", availability);
    query.bindValue(":publisherId", publisherId);
    query.bindValue(":authorId", authorId);
    query.bindValue(":categoryId", categoryId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Insert Error", query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "Book inserted successfully!");
        displayBooks();
        ui->lineEditBookName->clear();
        ui->lineEditNewPublisher->clear();
        ui->lineEditNewAuthorFirst->clear();
        ui->lineEditNewAuthorLast->clear();
        ui->lineEditNewCategory->clear();
    }
}

