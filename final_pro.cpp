#include <iostream>
#include <unordered_map>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <ctime>
using namespace std;
class User {
public:
    string name;
    int borrowedBooks;
    User() : name(""), borrowedBooks(0) {}  
    User(string n) : name(n), borrowedBooks(0) {}
};
class Book {
public:
    string title;
    string author;
    bool isBorrowed;
    queue<string> waitList;
    vector<int> ratings;
    time_t dueDate;
    Book() : title(""), author(""), isBorrowed(false), dueDate(0) {}
    Book(string t, string a) : title(t), author(a), isBorrowed(false), dueDate(0) {}
    void borrowBook(string userName) {
        if (!isBorrowed) {
            isBorrowed = true;
            dueDate = time(0) + 7 * 24 * 3600; 
            cout << userName << " has successfully borrowed \"" << title << "\". Due date: " << ctime(&dueDate) << endl;
        } else {
            cout << "\"" << title << "\" is already borrowed. Adding " << userName << " to the waiting list." << endl;
            waitList.push(userName);
        }
    }
    void returnBook() {
        if (isBorrowed) {
            isBorrowed = false;
            time_t now = time(0);
            if (now > dueDate) {
                double daysLate = difftime(now, dueDate) / (60 * 60 * 24);
                cout << "\"" << title << "\" is returned late. Penalty for " << daysLate << " days." << endl;
            }
            cout << "\"" << title << "\" has been returned." << endl;
            if (!waitList.empty()) {
                string nextUser = waitList.front();
                waitList.pop();
                borrowBook(nextUser);
            }
        } else {
            cout << "\"" << title << "\" was not borrowed." << endl;
        }
    }
    void rateBook(int rating) {
        ratings.push_back(rating);
        cout << "Thank you for rating \"" << title << "\"!" << endl;
    }
    void printRatings() {
        if (ratings.empty()) {
            cout << "No ratings yet for \"" << title << "\"." << endl;
        } else {
            double average = 0;
            for (int r : ratings) average += r;
            average /= ratings.size();
            cout << "Average rating for \"" << title << "\": " << average << "/5" << endl;
        }
    }
    void printWaitList() {
        if (waitList.empty()) {
            cout << "No one is waiting for \"" << title << "\"." << endl;
        } else {
            cout << "Waitlist for \"" << title << "\": ";
            queue<string> tempQueue = waitList;
            while (!tempQueue.empty()) {
                cout << tempQueue.front() << (tempQueue.size() > 1 ? ", " : "");
                tempQueue.pop();
            }
            cout << endl;
        }
    }
};
class Library {
private:
    unordered_map<string, Book> books;
    set<string> bookTitles;
    unordered_map<string, User> users;
public:
    void addUser(string userName) {
        if (users.find(userName) == users.end()) {
            users.emplace(userName, User(userName));
            cout << "User \"" << userName << "\" registered successfully." << endl;
        } else {
            cout << "User \"" << userName << "\" already exists." << endl;
        }
    }
    void addBook(string title, string author) {
        if (books.find(title) == books.end()) {
            books.emplace(title, Book(title, author));
            bookTitles.insert(title);
            cout << "Book \"" << title << "\" by " << author << " added to the library." << endl;
        } else {
            cout << "Book \"" << title << "\" already exists in the library." << endl;
        }
    }
    void searchBook(string title) {
        if (books.find(title) != books.end()) {
            Book &book = books[title];
            cout << "Book found: \"" << book.title << "\" by " << book.author;
            cout << (book.isBorrowed ? " (Borrowed)" : " (Available)") << endl;
            book.printRatings();
            book.printWaitList();
        } else {
            cout << "Book \"" << title << "\" not found in the library." << endl;
        }
    }
    void borrowBook(string title, string userName) {
        if (books.find(title) != books.end()) {
            Book &book = books[title];
            if (!book.isBorrowed || book.waitList.empty()) {
                users[userName].borrowedBooks++;
            }
            book.borrowBook(userName);
        } else {
            cout << "Book \"" << title << "\" not found in the library." << endl;
        }
    }
    void returnBook(string title, string userName) {
        if (books.find(title) != books.end()) {
            Book &book = books[title];
            if (book.isBorrowed) {
                users[userName].borrowedBooks--;
            }
            book.returnBook();
        } else {
            cout << "Book \"" << title << "\" not found in the library." << endl;
        }
    }
    void rateBook(string title, int rating) {
        if (books.find(title) != books.end()) {
            books[title].rateBook(rating);
        } else {
            cout << "Book \"" << title << "\" not found in the library." << endl;
        }
    }
   void listBooks() {
        if (bookTitles.empty()) {
            cout << "The library is empty." << endl;
        } else {
            cout << "Books in the library (sorted):" << endl;
            for (const auto &title : bookTitles) {
                Book &book = books[title];
                cout << "\"" << book.title << "\" by " << book.author;
                cout << (book.isBorrowed ? " (Borrowed)" : " (Available)") << endl;
                book.printRatings();
            }
        }
    }
};
int main() {
    Library library;
    int choice, rating;
    string title, author, userName;

    do {
        cout << "\nLibrary Management System\n";
        cout << "1. Add User\n";
        cout << "2. Add Book\n";
        cout << "3. Search Book\n";
        cout << "4. Borrow Book\n";
        cout << "5. Return Book\n";
        cout << "6. Rate Book\n";
        cout << "7. List All Books\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore();
        switch (choice) {
        case 1:
            cout << "Enter your name: ";
            getline(cin, userName);
            library.addUser(userName);
            break;
        case 2:
            cout << "Enter book title: ";
            getline(cin, title);
            cout << "Enter book author: ";
            getline(cin, author);
            library.addBook(title, author);
            break;
        case 3:
            cout << "Enter book title to search: ";
            getline(cin, title);
            library.searchBook(title);
            break;
        case 4:
            cout << "Enter your name: ";
            getline(cin, userName);
            cout << "Enter book title to borrow: ";
            getline(cin, title);
            library.borrowBook(title, userName);
            break;
        case 5:
            cout << "Enter your name: ";
            getline(cin, userName);
            cout << "Enter book title to return: ";
            getline(cin, title);
            library.returnBook(title, userName);
            break;
        case 6:
            cout << "Enter book title to rate: ";
            getline(cin, title);
            cout << "Enter rating (1-5): ";
            cin >> rating;
            library.rateBook(title, rating);
            break;
        case 7:
            library.listBooks();
            break;
        case 0:
            cout << "Exiting the system..." << endl;
            break;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 0);
    return 0;
}
