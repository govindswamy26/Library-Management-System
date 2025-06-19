 #include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

// Forward declarations
class Database;
class Book;
class User;
class Member;
class Admin;
class Transaction;
class Library;

// Database class to handle MySQL connections
class Database {
private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
    string server;
    string username;
    string password;
    string database;

public:
 Database::Database(const string& server, const string& username, 
                 const string& password, const string& database) 
    : server(server), username(username), password(password), database(database) {
    try {
        // Create connection
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
        
        // Verify connection
        if (!con->isValid()) {
            throw sql::SQLException("Connection is not valid");
        }
        
        // Select database
        con->setSchema(database);
        
        // Test connection with a simple query
        sql::Statement* stmt = con->createStatement();
        stmt->execute("SELECT 1");
        delete stmt;
        
        cout << "Successfully connected to MySQL database!" << endl;
    } catch (sql::SQLException &e) {
        cerr << "MySQL Connection Error:" << endl;
        cerr << "Error message: " << e.what() << endl;
        cerr << "Error code: " << e.getErrorCode() << endl;
        cerr << "SQLState: " << e.getSQLState() << endl;
        throw; // Re-throw to prevent invalid connections
    }
}

    ~Database() {
        if (con) {
            delete con;
        }
    }

    sql::Connection* getConnection() const {
        return con;
    }

    bool executeQuery(const string& query) {
        try {
            sql::Statement *stmt = con->createStatement();
            stmt->execute(query);
            delete stmt;
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Query Error: " << e.what() << endl;
            return false;
        }
    }

    sql::ResultSet* executeQueryWithResults(const string& query) {
        try {
            sql::Statement *stmt = con->createStatement();
            sql::ResultSet *res = stmt->executeQuery(query);
            // Note: Caller must delete the ResultSet and Statement
            return res;
        } catch (sql::SQLException &e) {
            cerr << "Query Error: " << e.what() << endl;
            return nullptr;
        }
    }
};

// Book class representing a book entity
class Book {
private:
    int book_id;
    string title;
    string author;
    string publisher;
    int year;
    bool available;

public:
    Book(int id, const string& title, const string& author, 
         const string& publisher, int year, bool available)
        : book_id(id), title(title), author(author), 
          publisher(publisher), year(year), available(available) {}

    // Getters
    int getBookId() const { return book_id; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    int getYear() const { return year; }
    bool isAvailable() const { return available; }

    // Setters
    void setTitle(const string& newTitle) { title = newTitle; }
    void setAuthor(const string& newAuthor) { author = newAuthor; }
    void setPublisher(const string& newPublisher) { publisher = newPublisher; }
    void setYear(int newYear) { year = newYear; }
    void setAvailable(bool status) { available = status; }

    void display() const {
        cout << "ID: " << book_id << "\nTitle: " << title 
             << "\nAuthor: " << author << "\nPublisher: " << publisher
             << "\nYear: " << year << "\nAvailable: " << (available ? "Yes" : "No") << endl;
    }
};

// User base class
class User {
protected:
    int user_id;
    string name;
    string email;
    string password;
    string role;

public:
    User(int id, const string& name, const string& email, 
         const string& password, const string& role)
        : user_id(id), name(name), email(email), password(password), role(role) {}

    // Getters
    int getUserId() const { return user_id; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getRole() const { return role; }

    // Authentication
    virtual bool authenticate(const string& inputPassword) const {
        return password == inputPassword;
    }

    virtual void display() const {
        cout << "ID: " << user_id << "\nName: " << name 
             << "\nEmail: " << email << "\nRole: " << role << endl;
    }

    virtual ~User() {}
};

// Member class derived from User
class Member : public User {
private:
    int borrow_limit;
    vector<int> borrowed_books;

public:
    Member(int id, const string& name, const string& email, 
           const string& password, int borrow_limit = 5)
        : User(id, name, email, password, "member"), borrow_limit(borrow_limit) {}

    // Getter
    int getBorrowLimit() const { return borrow_limit; }
    const vector<int>& getBorrowedBooks() const { return borrowed_books; }

    // Book management
    bool canBorrow() const {
        return borrowed_books.size() < borrow_limit;
    }

    void addBorrowedBook(int book_id) {
        borrowed_books.push_back(book_id);
    }

    bool returnBook(int book_id) {
        auto it = find(borrowed_books.begin(), borrowed_books.end(), book_id);
        if (it != borrowed_books.end()) {
            borrowed_books.erase(it);
            return true;
        }
        return false;
    }

    void display() const override {
        User::display();
        cout << "Borrowed Books (" << borrowed_books.size() << "/" << borrow_limit << "): ";
        for (int book_id : borrowed_books) {
            cout << book_id << " ";
        }
        cout << endl;
    }
};

// Admin class derived from User
class Admin : public User {
public:
    Admin(int id, const string& name, const string& email, const string& password)
        : User(id, name, email, password, "admin") {}

    void display() const override {
        User::display();
        cout << "Administrator privileges" << endl;
    }
};

// Transaction class
class Transaction {
private:
    int transaction_id;
    int book_id;
    int user_id;
    string issue_date;
    string return_date;
    string status; // "issued", "returned", "overdue"

public:
    Transaction(int id, int book_id, int user_id, 
                const string& issue_date, const string& return_date, 
                const string& status)
        : transaction_id(id), book_id(book_id), user_id(user_id),
          issue_date(issue_date), return_date(return_date), status(status) {}

    // Getters
    int getTransactionId() const { return transaction_id; }
    int getBookId() const { return book_id; }
    int getUserId() const { return user_id; }
    string getIssueDate() const { return issue_date; }
    string getReturnDate() const { return return_date; }
    string getStatus() const { return status; }

    // Setters
    void setReturnDate(const string& date) { return_date = date; }
    void setStatus(const string& newStatus) { status = newStatus; }

    void display() const {
        cout << "Transaction ID: " << transaction_id 
             << "\nBook ID: " << book_id 
             << "\nUser ID: " << user_id
             << "\nIssue Date: " << issue_date
             << "\nReturn Date: " << return_date
             << "\nStatus: " << status << endl;
    }
};

// Library class - main system class
class Library {
private:
    Database* db;
    vector<Book> books;
    vector<User*> users;
    vector<Transaction> transactions;

    // Private helper methods
    void loadBooksFromDB() {
        books.clear();
        sql::ResultSet* res = db->executeQueryWithResults("SELECT * FROM books");
        if (res) {
            while (res->next()) {
                Book book(
                    res->getInt("book_id"),
                    res->getString("title"),
                    res->getString("author"),
                    res->getString("publisher"),
                    res->getInt("year"),
                    res->getBoolean("available")
                );
                books.push_back(book);
            }
            delete res;
        }
    }

    void loadUsersFromDB() {
        for (auto user : users) {
            delete user;
        }
        users.clear();

        sql::ResultSet* res = db->executeQueryWithResults("SELECT * FROM users");
        if (res) {
            while (res->next()) {
                string role = res->getString("role");
                if (role == "admin") {
                    users.push_back(new Admin(
                        res->getInt("user_id"),
                        res->getString("name"),
                        res->getString("email"),
                        res->getString("password")
                    ));
                } else { // member
                    users.push_back(new Member(
                        res->getInt("user_id"),
                        res->getString("name"),
                        res->getString("email"),
                        res->getString("password")
                    ));
                }
            }
            delete res;
        }
    }

    void loadTransactionsFromDB() {
        transactions.clear();
        sql::ResultSet* res = db->executeQueryWithResults("SELECT * FROM transactions");
        if (res) {
            while (res->next()) {
                Transaction trans(
                    res->getInt("transaction_id"),
                    res->getInt("book_id"),
                    res->getInt("user_id"),
                    res->getString("issue_date"),
                    res->getString("return_date"),
                    res->getString("status")
                );
                transactions.push_back(trans);
            }
            delete res;
        }
    }

    string getCurrentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return to_string(1900 + ltm->tm_year) + "-" + 
               to_string(1 + ltm->tm_mon) + "-" + 
               to_string(ltm->tm_mday);
    }

    string calculateReturnDate() {
        time_t now = time(0);
        now += 14 * 24 * 60 * 60; // Add 14 days
        tm* ltm = localtime(&now);
        return to_string(1900 + ltm->tm_year) + "-" + 
               to_string(1 + ltm->tm_mon) + "-" + 
               to_string(ltm->tm_mday);
    }

public:
    Library(Database* database) : db(database) {
        loadBooksFromDB();
        loadUsersFromDB();
        loadTransactionsFromDB();
    }

    ~Library() {
        for (auto user : users) {
            delete user;
        }
    }

    // Book management
    bool addBook(const string& title, const string& author, 
                 const string& publisher, int year) {
        try {
            sql::PreparedStatement* pstmt = db->getConnection()->prepareStatement(
                "INSERT INTO books (title, author, publisher, year, available) VALUES (?, ?, ?, ?, ?)");
            pstmt->setString(1, title);
            pstmt->setString(2, author);
            pstmt->setString(3, publisher);
            pstmt->setInt(4, year);
            pstmt->setBoolean(5, true);
            pstmt->executeUpdate();
            delete pstmt;
            
            loadBooksFromDB(); // Refresh the books list
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error adding book: " << e.what() << endl;
            return false;
        }
    }

    bool updateBook(int book_id, const string& title, const string& author, 
                   const string& publisher, int year, bool available) {
        try {
            sql::PreparedStatement* pstmt = db->getConnection()->prepareStatement(
                "UPDATE books SET title=?, author=?, publisher=?, year=?, available=? WHERE book_id=?");
            pstmt->setString(1, title);
            pstmt->setString(2, author);
            pstmt->setString(3, publisher);
            pstmt->setInt(4, year);
            pstmt->setBoolean(5, available);
            pstmt->setInt(6, book_id);
            pstmt->executeUpdate();
            delete pstmt;
            
            loadBooksFromDB(); // Refresh the books list
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error updating book: " << e.what() << endl;
            return false;
        }
    }

    bool deleteBook(int book_id) {
        try {
            sql::PreparedStatement* pstmt = db->getConnection()->prepareStatement(
                "DELETE FROM books WHERE book_id=?");
            pstmt->setInt(1, book_id);
            pstmt->executeUpdate();
            delete pstmt;
            
            loadBooksFromDB(); // Refresh the books list
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error deleting book: " << e.what() << endl;
            return false;
        }
    }

    Book* findBookById(int book_id) {
        for (auto& book : books) {
            if (book.getBookId() == book_id) {
                return &book;
            }
        }
        return nullptr;
    }

    vector<Book> searchBooks(const string& keyword) {
        vector<Book> results;
        string lowerKeyword = keyword;
        transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);

        for (const auto& book : books) {
            string lowerTitle = book.getTitle();
            transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
            
            string lowerAuthor = book.getAuthor();
            transform(lowerAuthor.begin(), lowerAuthor.end(), lowerAuthor.begin(), ::tolower);

            if (lowerTitle.find(lowerKeyword) != string::npos || 
                lowerAuthor.find(lowerKeyword) != string::npos) {
                results.push_back(book);
            }
        }
        return results;
    }

    // User management
    User* authenticateUser(const string& email, const string& password) {
        for (auto user : users) {
            if (user->getEmail() == email && user->authenticate(password)) {
                return user;
            }
        }
        return nullptr;
    }

    bool addUser(const string& name, const string& email, 
                 const string& password, const string& role) {
        try {
            sql::PreparedStatement* pstmt = db->getConnection()->prepareStatement(
                "INSERT INTO users (name, email, password, role) VALUES (?, ?, ?, ?)");
            pstmt->setString(1, name);
            pstmt->setString(2, email);
            pstmt->setString(3, password);
            pstmt->setString(4, role);
            pstmt->executeUpdate();
            delete pstmt;
            
            loadUsersFromDB(); // Refresh the users list
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error adding user: " << e.what() << endl;
            return false;
        }
    }

    bool deleteUser(int user_id) {
        try {
            sql::PreparedStatement* pstmt = db->getConnection()->prepareStatement(
                "DELETE FROM users WHERE user_id=?");
            pstmt->setInt(1, user_id);
            pstmt->executeUpdate();
            delete pstmt;
            
            loadUsersFromDB(); // Refresh the users list
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error deleting user: " << e.what() << endl;
            return false;
        }
    }

    User* findUserById(int user_id) {
        for (auto user : users) {
            if (user->getUserId() == user_id) {
                return user;
            }
        }
        return nullptr;
    }

    // Transaction management
    bool issueBook(int book_id, int user_id) {
        Book* book = findBookById(book_id);
        User* user = findUserById(user_id);
        
        if (!book || !user) {
            cerr << "Invalid book or user ID" << endl;
            return false;
        }
        
        if (!book->isAvailable()) {
            cerr << "Book is not available" << endl;
            return false;
        }
        
        if (user->getRole() == "member") {
            Member* member = dynamic_cast<Member*>(user);
            if (!member->canBorrow()) {
                cerr << "Member has reached borrow limit" << endl;
                return false;
            }
        }
        
        try {
            // Start transaction
            db->getConnection()->setAutoCommit(false);
            
            // Update book availability
            sql::PreparedStatement* pstmt1 = db->getConnection()->prepareStatement(
                "UPDATE books SET available=false WHERE book_id=?");
            pstmt1->setInt(1, book_id);
            pstmt1->executeUpdate();
            delete pstmt1;
            
            // Add transaction record
            string issue_date = getCurrentDate();
            string return_date = calculateReturnDate();
            
            sql::PreparedStatement* pstmt2 = db->getConnection()->prepareStatement(
                "INSERT INTO transactions (book_id, user_id, issue_date, return_date, status) VALUES (?, ?, ?, ?, ?)");
            pstmt2->setInt(1, book_id);
            pstmt2->setInt(2, user_id);
            pstmt2->setString(3, issue_date);
            pstmt2->setString(4, return_date);
            pstmt2->setString(5, "issued");
            pstmt2->executeUpdate();
            delete pstmt2;
            
            // If member, update their borrowed books list
            if (user->getRole() == "member") {
                Member* member = dynamic_cast<Member*>(user);
                member->addBorrowedBook(book_id);
            }
            
            // Commit transaction
            db->getConnection()->commit();
            db->getConnection()->setAutoCommit(true);
            
            // Refresh data
            loadBooksFromDB();
            loadTransactionsFromDB();
            
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error issuing book: " << e.what() << endl;
            try {
                db->getConnection()->rollback();
                db->getConnection()->setAutoCommit(true);
            } catch (sql::SQLException &e2) {
                cerr << "Error on rollback: " << e2.what() << endl;
            }
            return false;
        }
    }

    bool returnBook(int book_id, int user_id) {
        Book* book = findBookById(book_id);
        User* user = findUserById(user_id);
        
        if (!book || !user) {
            cerr << "Invalid book or user ID" << endl;
            return false;
        }
        
        if (book->isAvailable()) {
            cerr << "Book is already available" << endl;
            return false;
        }
        
        try {
            // Start transaction
            db->getConnection()->setAutoCommit(false);
            
            // Update book availability
            sql::PreparedStatement* pstmt1 = db->getConnection()->prepareStatement(
                "UPDATE books SET available=true WHERE book_id=?");
            pstmt1->setInt(1, book_id);
            pstmt1->executeUpdate();
            delete pstmt1;
            
            // Update transaction record
            string return_date = getCurrentDate();
            
            sql::PreparedStatement* pstmt2 = db->getConnection()->prepareStatement(
                "UPDATE transactions SET return_date=?, status='returned' WHERE book_id=? AND user_id=? AND status='issued'");
            pstmt2->setString(1, return_date);
            pstmt2->setInt(2, book_id);
            pstmt2->setInt(3, user_id);
            pstmt2->executeUpdate();
            delete pstmt2;
            
            // If member, update their borrowed books list
            if (user->getRole() == "member") {
                Member* member = dynamic_cast<Member*>(user);
                member->returnBook(book_id);
            }
            
            // Commit transaction
            db->getConnection()->commit();
            db->getConnection()->setAutoCommit(true);
            
            // Refresh data
            loadBooksFromDB();
            loadTransactionsFromDB();
            
            return true;
        } catch (sql::SQLException &e) {
            cerr << "Error returning book: " << e.what() << endl;
            try {
                db->getConnection()->rollback();
                db->getConnection()->setAutoCommit(true);
            } catch (sql::SQLException &e2) {
                cerr << "Error on rollback: " << e2.what() << endl;
            }
            return false;
        }
    }

    vector<Transaction> getUserTransactions(int user_id) {
        vector<Transaction> user_transactions;
        for (const auto& trans : transactions) {
            if (trans.getUserId() == user_id) {
                user_transactions.push_back(trans);
            }
        }
        return user_transactions;
    }

    vector<Transaction> getBookTransactions(int book_id) {
        vector<Transaction> book_transactions;
        for (const auto& trans : transactions) {
            if (trans.getBookId() == book_id) {
                book_transactions.push_back(trans);
            }
        }
        return book_transactions;
    }

    // Reporting
    void displayAllBooks() const {
        cout << "\n=== All Books ===\n";
        for (const auto& book : books) {
            book.display();
            cout << "-----------------\n";
        }
    }

    void displayAllUsers() const {
        cout << "\n=== All Users ===\n";
        for (const auto& user : users) {
            user->display();
            cout << "-----------------\n";
        }
    }

    void displayAllTransactions() const {
        cout << "\n=== All Transactions ===\n";
        for (const auto& trans : transactions) {
            trans.display();
            cout << "-----------------\n";
        }
    }

    void displayOverdueBooks() {
        string current_date = getCurrentDate();
        
        try {
            sql::ResultSet* res = db->executeQueryWithResults(
                "SELECT t.*, b.title, u.name FROM transactions t "
                "JOIN books b ON t.book_id = b.book_id "
                "JOIN users u ON t.user_id = u.user_id "
                "WHERE t.status = 'issued' AND t.return_date < '" + current_date + "'");
            
            if (res) {
                cout << "\n=== Overdue Books ===\n";
                while (res->next()) {
                    cout << "Transaction ID: " << res->getInt("transaction_id") << "\n"
                         << "Book: " << res->getString("title") << " (ID: " << res->getInt("book_id") << ")\n"
                         << "User: " << res->getString("name") << " (ID: " << res->getInt("user_id") << ")\n"
                         << "Issue Date: " << res->getString("issue_date") << "\n"
                         << "Due Date: " << res->getString("return_date") << "\n"
                         << "-----------------\n";
                }
                delete res;
            }
        } catch (sql::SQLException &e) {
            cerr << "Error fetching overdue books: " << e.what() << endl;
        }
    }
};

// Utility functions
void displayMenu() {
    cout << "\n=== Library Management System ===\n";
    cout << "1. Login\n";
    cout << "2. Exit\n";
    cout << "Enter your choice: ";
}

void displayMemberMenu() {
    cout << "\n=== Member Menu ===\n";
    cout << "1. View All Books\n";
    cout << "2. Search Books\n";
    cout << "3. View My Borrowed Books\n";
    cout << "4. View My Transactions\n";
    cout << "5. Logout\n";
    cout << "Enter your choice: ";
}

void displayAdminMenu() {
    cout << "\n=== Admin Menu ===\n";
    cout << "1. View All Books\n";
    cout << "2. Add Book\n";
    cout << "3. Update Book\n";
    cout << "4. Delete Book\n";
    cout << "5. Search Books\n";
    cout << "6. View All Users\n";
    cout << "7. Add User\n";
    cout << "8. Delete User\n";
    cout << "9. View All Transactions\n";
    cout << "10. View Overdue Books\n";
    cout << "11. Issue Book\n";
    cout << "12. Return Book\n";
    cout << "13. Logout\n";
    cout << "Enter your choice: ";
}

// Main function
int main() {
    // Initialize database connection
   try {
        // Connection parameters
        string server = "tcp://127.0.0.1:3306"; // or "localhost"
        string username = "root";
        string password = "2603"; // Use your MySQL root password
        string database = "library_db";
        
        // Create database connection
        Database db(server, username, password, database);
        
        // Create library instance
        Library library(&db);
        
        // Rest of your application code...
        
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    // Create Library instance
    Library library(&db);
    
    User* currentUser = nullptr;
    
    while (true) {
        if (!currentUser) {
            // Main menu
            displayMenu();
            int choice;
            cin >> choice;
            cin.ignore(); // Clear newline
            
            if (choice == 1) {
                // Login
                string email, password;
                cout << "Enter email: ";
                getline(cin, email);
                cout << "Enter password: ";
                getline(cin, password);
                
                currentUser = library.authenticateUser(email, password);
                if (!currentUser) {
                    cout << "Invalid credentials or user not found.\n";
                } else {
                    cout << "Login successful! Welcome, " << currentUser->getName() << "!\n";
                }
            } else if (choice == 2) {
                // Exit
                cout << "Goodbye!\n";
                break;
            } else {
                cout << "Invalid choice. Please try again.\n";
            }
        } else {
            // Role-based menu
            if (currentUser->getRole() == "member") {
                Member* member = dynamic_cast<Member*>(currentUser);
                
                displayMemberMenu();
                int choice;
                cin >> choice;
                cin.ignore(); // Clear newline
                
                if (choice == 1) {
                    // View all books
                    library.displayAllBooks();
                } else if (choice == 2) {
                    // Search books
                    string keyword;
                    cout << "Enter search keyword: ";
                    getline(cin, keyword);
                    
                    vector<Book> results = library.searchBooks(keyword);
                    cout << "\n=== Search Results ===\n";
                    for (const auto& book : results) {
                        book.display();
                        cout << "-----------------\n";
                    }
                } else if (choice == 3) {
                    // View borrowed books
                    cout << "\n=== Your Borrowed Books ===\n";
                    const vector<int>& borrowed = member->getBorrowedBooks();
                    for (int book_id : borrowed) {
                        Book* book = library.findBookById(book_id);
                        if (book) {
                            book->display();
                            cout << "-----------------\n";
                        }
                    }
                    if (borrowed.empty()) {
                        cout << "You have no borrowed books.\n";
                    }
                } else if (choice == 4) {
                    // View transactions
                    vector<Transaction> trans = library.getUserTransactions(member->getUserId());
                    cout << "\n=== Your Transactions ===\n";
                    for (const auto& t : trans) {
                        t.display();
                        cout << "-----------------\n";
                    }
                    if (trans.empty()) {
                        cout << "No transactions found.\n";
                    }
                } else if (choice == 5) {
                    // Logout
                    currentUser = nullptr;
                    cout << "Logged out successfully.\n";
                } else {
                    cout << "Invalid choice. Please try again.\n";
                }
            } else if (currentUser->getRole() == "admin") {
                displayAdminMenu();
                int choice;
                cin >> choice;
                cin.ignore(); // Clear newline
                
                if (choice == 1) {
                    // View all books
                    library.displayAllBooks();
                } else if (choice == 2) {
                    // Add book
                    string title, author, publisher;
                    int year;
                    
                    cout << "Enter book title: ";
                    getline(cin, title);
                    cout << "Enter author: ";
                    getline(cin, author);
                    cout << "Enter publisher: ";
                    getline(cin, publisher);
                    cout << "Enter publication year: ";
                    cin >> year;
                    cin.ignore(); // Clear newline
                    
                    if (library.addBook(title, author, publisher, year)) {
                        cout << "Book added successfully!\n";
                    } else {
                        cout << "Failed to add book.\n";
                    }
                } else if (choice == 3) {
                    // Update book
                    int book_id;
                    library.displayAllBooks();
                    cout << "Enter book ID to update: ";
                    cin >> book_id;
                    cin.ignore(); // Clear newline
                    
                    Book* book = library.findBookById(book_id);
                    if (!book) {
                        cout << "Book not found.\n";
                        continue;
                    }
                    
                    string title, author, publisher;
                    int year;
                    bool available;
                    
                    cout << "Enter new title (current: " << book->getTitle() << "): ";
                    getline(cin, title);
                    cout << "Enter new author (current: " << book->getAuthor() << "): ";
                    getline(cin, author);
                    cout << "Enter new publisher (current: " << book->getPublisher() << "): ";
                    getline(cin, publisher);
                    cout << "Enter new publication year (current: " << book->getYear() << "): ";
                    cin >> year;
                    cout << "Is available? (1 for yes, 0 for no) (current: " << book->isAvailable() << "): ";
                    cin >> available;
                    cin.ignore(); // Clear newline
                    
                    if (library.updateBook(book_id, title, author, publisher, year, available)) {
                        cout << "Book updated successfully!\n";
                    } else {
                        cout << "Failed to update book.\n";
                    }
                } else if (choice == 4) {
                    // Delete book
                    library.displayAllBooks();
                    int book_id;
                    cout << "Enter book ID to delete: ";
                    cin >> book_id;
                    cin.ignore(); // Clear newline
                    
                    if (library.deleteBook(book_id)) {
                        cout << "Book deleted successfully!\n";
                    } else {
                        cout << "Failed to delete book.\n";
                    }
                } else if (choice == 5) {
                    // Search books
                    string keyword;
                    cout << "Enter search keyword: ";
                    getline(cin, keyword);
                    
                    vector<Book> results = library.searchBooks(keyword);
                    cout << "\n=== Search Results ===\n";
                    for (const auto& book : results) {
                        book.display();
                        cout << "-----------------\n";
                    }
                } else if (choice == 6) {
                    // View all users
                    library.displayAllUsers();
                } else if (choice == 7) {
                    // Add user
                    string name, email, password, role;
                    
                    cout << "Enter user name: ";
                    getline(cin, name);
                    cout << "Enter email: ";
                    getline(cin, email);
                    cout << "Enter password: ";
                    getline(cin, password);
                    cout << "Enter role (admin/member): ";
                    getline(cin, role);
                    
                    if (role != "admin" && role != "member") {
                        cout << "Invalid role. Must be 'admin' or 'member'.\n";
                        continue;
                    }
                    
                    if (library.addUser(name, email, password, role)) {
                        cout << "User added successfully!\n";
                    } else {
                        cout << "Failed to add user.\n";
                    }
                } else if (choice == 8) {
                    // Delete user
                    library.displayAllUsers();
                    int user_id;
                    cout << "Enter user ID to delete: ";
                    cin >> user_id;
                    cin.ignore(); // Clear newline
                    
                    if (library.deleteUser(user_id)) {
                        cout << "User deleted successfully!\n";
                    } else {
                        cout << "Failed to delete user.\n";
                    }
                } else if (choice == 9) {
                    // View all transactions
                    library.displayAllTransactions();
                } else if (choice == 10) {
                    // View overdue books
                    library.displayOverdueBooks();
                } else if (choice == 11) {
                    // Issue book
                    library.displayAllBooks();
                    int book_id, user_id;
                    
                    cout << "Enter book ID to issue: ";
                    cin >> book_id;
                    cout << "Enter user ID: ";
                    cin >> user_id;
                    cin.ignore(); // Clear newline
                    
                    if (library.issueBook(book_id, user_id)) {
                        cout << "Book issued successfully!\n";
                    } else {
                        cout << "Failed to issue book.\n";
                    }
                } else if (choice == 12) {
                    // Return book
                    int book_id, user_id;
                    
                    cout << "Enter book ID to return: ";
                    cin >> book_id;
                    cout << "Enter user ID: ";
                    cin >> user_id;
                    cin.ignore(); // Clear newline
                    
                    if (library.returnBook(book_id, user_id)) {
                        cout << "Book returned successfully!\n";
                    } else {
                        cout << "Failed to return book.\n";
                    }
                } else if (choice == 13) {
                    // Logout
                    currentUser = nullptr;
                    cout << "Logged out successfully.\n";
                } else {
                    cout << "Invalid choice. Please try again.\n";
                }
            }
        }
    }
    
    return 0;
}
