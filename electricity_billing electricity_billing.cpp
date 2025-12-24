#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>
#include <ctime>
#include <sstream>

using namespace std;

// Structure to store customer information
struct Customer {
    int customerID;
    string name;
    string address;
    string contact;
    double previousReading;
    double currentReading;
    double unitsConsumed;
    double billAmount;
    string billingDate;
    bool isPaid;
    
    Customer() : customerID(0), previousReading(0.0), currentReading(0.0), 
                 unitsConsumed(0.0), billAmount(0.0), isPaid(false) {}
};

// Structure for tariff rates
struct Tariff {
    double domesticRate;
    double commercialRate;
    double industrialRate;
    
    Tariff() : domesticRate(5.0), commercialRate(7.5), industrialRate(10.0) {}
};

// Global variables
vector<Customer> customers;
Tariff currentTariff;
const string DATA_FILE = "customers.dat";
const string TARIFF_FILE = "tariff.dat";

// Function prototypes
void displayMenu();
void addCustomer();
void calculateBill(Customer &customer);
void generateBill();
void viewAllCustomers();
void searchCustomer();
void updateCustomer();
void deleteCustomer();
void payBill();
void viewPaidBills();
void viewPendingBills();
void updateTariff();
void generateReport();
void saveData();
void loadData();
void saveTariff();
void loadTariff();
string getCurrentDate();
int generateCustomerID();
void clearScreen();
void pressEnterToContinue();
double getValidDouble(const string &prompt);
int getValidInt(const string &prompt);

int main() {
    loadData();
    loadTariff();
    
    int choice;
    do {
        clearScreen();
        displayMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        
        // Clear input buffer
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch(choice) {
            case 1:
                addCustomer();
                break;
            case 2:
                generateBill();
                break;
            case 3:
                viewAllCustomers();
                break;
            case 4:
                searchCustomer();
                break;
            case 5:
                updateCustomer();
                break;
            case 6:
                deleteCustomer();
                break;
            case 7:
                payBill();
                break;
            case 8:
                viewPaidBills();
                break;
            case 9:
                viewPendingBills();
                break;
            case 10:
                updateTariff();
                break;
            case 11:
                generateReport();
                break;
            case 12:
                saveData();
                saveTariff();
                cout << "\nData saved successfully. Exiting...\n";
                break;
            default:
                if (choice != 12) {
                    cout << "\nInvalid choice! Please try again.\n";
                    pressEnterToContinue();
                }
        }
    } while (choice != 12);
    
    return 0;
}

void displayMenu() {
    cout << "=========================================\n";
    cout << "   ELECTRICITY BILLING SYSTEM\n";
    cout << "=========================================\n";
    cout << "1. Add New Customer\n";
    cout << "2. Generate Electricity Bill\n";
    cout << "3. View All Customers\n";
    cout << "4. Search Customer\n";
    cout << "5. Update Customer Details\n";
    cout << "6. Delete Customer\n";
    cout << "7. Pay Bill\n";
    cout << "8. View Paid Bills\n";
    cout << "9. View Pending Bills\n";
    cout << "10. Update Tariff Rates\n";
    cout << "11. Generate Report\n";
    cout << "12. Exit and Save Data\n";
    cout << "=========================================\n";
}

void addCustomer() {
    clearScreen();
    cout << "=== ADD NEW CUSTOMER ===\n\n";
    
    Customer newCustomer;
    newCustomer.customerID = generateCustomerID();
    
    cout << "Customer ID: " << newCustomer.customerID << endl;
    
    cout << "Enter Customer Name: ";
    getline(cin, newCustomer.name);
    
    cout << "Enter Address: ";
    getline(cin, newCustomer.address);
    
    cout << "Enter Contact Number: ";
    getline(cin, newCustomer.contact);
    
    newCustomer.previousReading = getValidDouble("Enter Previous Meter Reading: ");
    newCustomer.currentReading = getValidDouble("Enter Current Meter Reading: ");
    
    // Calculate initial bill
    calculateBill(newCustomer);
    
    customers.push_back(newCustomer);
    
    cout << "\nCustomer added successfully!\n";
    cout << "Generated Customer ID: " << newCustomer.customerID << endl;
    
    pressEnterToContinue();
}

void calculateBill(Customer &customer) {
    customer.unitsConsumed = customer.currentReading - customer.previousReading;
    
    // Apply tariff (using domestic rate as default for simplicity)
    // In a real system, you would ask for customer type
    customer.billAmount = customer.unitsConsumed * currentTariff.domesticRate;
    
    // Add fixed charges and tax (simplified)
    double fixedCharge = 50.0; // Fixed monthly charge
    double taxRate = 0.18; // 18% tax
    
    customer.billAmount += fixedCharge;
    customer.billAmount += customer.billAmount * taxRate;
    
    customer.billingDate = getCurrentDate();
    customer.isPaid = false;
}

void generateBill() {
    clearScreen();
    cout << "=== GENERATE ELECTRICITY BILL ===\n\n";
    
    if (customers.empty()) {
        cout << "No customers found! Please add customers first.\n";
        pressEnterToContinue();
        return;
    }
    
    int id = getValidInt("Enter Customer ID: ");
    
    // Search for customer
    auto it = find_if(customers.begin(), customers.end(), 
                     [id](const Customer &c) { return c.customerID == id; });
    
    if (it == customers.end()) {
        cout << "Customer not found with ID: " << id << endl;
        pressEnterToContinue();
        return;
    }
    
    cout << "\nEnter new meter reading for billing:\n";
    it->previousReading = it->currentReading;
    it->currentReading = getValidDouble("Enter Current Meter Reading: ");
    
    calculateBill(*it);
    
    // Display the bill
    clearScreen();
    cout << "=========================================\n";
    cout << "        ELECTRICITY BILL\n";
    cout << "=========================================\n";
    cout << "Bill Date: " << it->billingDate << endl;
    cout << "Customer ID: " << it->customerID << endl;
    cout << "Customer Name: " << it->name << endl;
    cout << "Address: " << it->address << endl;
    cout << "Contact: " << it->contact << endl;
    cout << "-----------------------------------------\n";
    cout << fixed << setprecision(2);
    cout << "Previous Reading: " << it->previousReading << " units\n";
    cout << "Current Reading: " << it->currentReading << " units\n";
    cout << "Units Consumed: " << it->unitsConsumed << " units\n";
    cout << "-----------------------------------------\n";
    cout << "Bill Amount: Rs. " << it->billAmount << endl;
    cout << "Payment Status: " << (it->isPaid ? "PAID" : "PENDING") << endl;
    cout << "=========================================\n";
    
    pressEnterToContinue();
}

void viewAllCustomers() {
    clearScreen();
    cout << "=== ALL CUSTOMERS ===\n\n";
    
    if (customers.empty()) {
        cout << "No customers found!\n";
        pressEnterToContinue();
        return;
    }
    
    cout << left << setw(10) << "ID" 
         << setw(20) << "Name" 
         << setw(15) << "Contact" 
         << setw(12) << "Units Used" 
         << setw(12) << "Bill Amount" 
         << setw(10) << "Status" << endl;
    cout << string(80, '-') << endl;
    
    cout << fixed << setprecision(2);
    for (const auto &customer : customers) {
        cout << left << setw(10) << customer.customerID
             << setw(20) << (customer.name.length() > 18 ? customer.name.substr(0, 15) + "..." : customer.name)
             << setw(15) << customer.contact
             << setw(12) << customer.unitsConsumed
             << setw(12) << customer.billAmount
             << setw(10) << (customer.isPaid ? "PAID" : "PENDING") << endl;
    }
    
    pressEnterToContinue();
}

void searchCustomer() {
    clearScreen();
    cout << "=== SEARCH CUSTOMER ===\n\n";
    
    if (customers.empty()) {
        cout << "No customers found!\n";
        pressEnterToContinue();
        return;
    }
    
    int choice;
    cout << "Search by:\n";
    cout << "1. Customer ID\n";
    cout << "2. Customer Name\n";
    cout << "Enter choice: ";
    cin >> choice;
    cin.ignore();
    
    if (choice == 1) {
        int id = getValidInt("Enter Customer ID: ");
        
        auto it = find_if(customers.begin(), customers.end(),
                         [id](const Customer &c) { return c.customerID == id; });
        
        if (it != customers.end()) {
            clearScreen();
            cout << "=== CUSTOMER DETAILS ===\n\n";
            cout << "Customer ID: " << it->customerID << endl;
            cout << "Name: " << it->name << endl;
            cout << "Address: " << it->address << endl;
            cout << "Contact: " << it->contact << endl;
            cout << fixed << setprecision(2);
            cout << "Previous Reading: " << it->previousReading << " units\n";
            cout << "Current Reading: " << it->currentReading << " units\n";
            cout << "Units Consumed: " << it->unitsConsumed << " units\n";
            cout << "Bill Amount: Rs. " << it->billAmount << endl;
            cout << "Billing Date: " << it->billingDate << endl;
            cout << "Payment Status: " << (it->isPaid ? "PAID" : "PENDING") << endl;
        } else {
            cout << "Customer not found with ID: " << id << endl;
        }
    } else if (choice == 2) {
        string name;
        cout << "Enter Customer Name (or part): ";
        getline(cin, name);
        
        // Convert to lowercase for case-insensitive search
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        
        bool found = false;
        for (const auto &customer : customers) {
            string customerNameLower = customer.name;
            transform(customerNameLower.begin(), customerNameLower.end(), customerNameLower.begin(), ::tolower);
            
            if (customerNameLower.find(name) != string::npos) {
                if (!found) {
                    clearScreen();
                    cout << "=== SEARCH RESULTS ===\n\n";
                    found = true;
                }
                cout << "ID: " << customer.customerID 
                     << " | Name: " << customer.name 
                     << " | Contact: " << customer.contact 
                     << " | Bill: Rs. " << fixed << setprecision(2) << customer.billAmount
                     << " | Status: " << (customer.isPaid ? "PAID" : "PENDING") << endl;
            }
        }
        
        if (!found) {
            cout << "No customers found with name containing: " << name << endl;
        }
    } else {
        cout << "Invalid choice!\n";
    }
    
    pressEnterToContinue();
}

void updateCustomer() {
    clearScreen();
    cout << "=== UPDATE CUSTOMER DETAILS ===\n\n";
    
    if (customers.empty()) {
        cout << "No customers found!\n";
        pressEnterToContinue();
        return;
    }
    
    int id = getValidInt("Enter Customer ID to update: ");
    
    auto it = find_if(customers.begin(), customers.end(),
                     [id](const Customer &c) { return c.customerID == id; });
    
    if (it == customers.end()) {
        cout << "Customer not found with ID: " << id << endl;
        pressEnterToContinue();
        return;
    }
    
    cout << "\nCurrent Details:\n";
    cout << "1. Name: " << it->name << endl;
    cout << "2. Address: " << it->address << endl;
    cout << "3. Contact: " << it->contact << endl;
    cout << "4. Previous Reading: " << it->previousReading << endl;
    cout << "5. Current Reading: " << it->currentReading << endl;
    
    int choice;
    cout << "\nSelect field to update (1-5, 0 to cancel): ";
    cin >> choice;
    cin.ignore();
    
    switch(choice) {
        case 1:
            cout << "Enter new Name: ";
            getline(cin, it->name);
            break;
        case 2:
            cout << "Enter new Address: ";
            getline(cin, it->address);
            break;
        case 3:
            cout << "Enter new Contact: ";
            getline(cin, it->contact);
            break;
        case 4:
            it->previousReading = getValidDouble("Enter new Previous Reading: ");
            break;
        case 5:
            it->currentReading = getValidDouble("Enter new Current Reading: ");
            break;
        case 0:
            cout << "Update cancelled.\n";
            break;
        default:
            cout << "Invalid choice!\n";
    }
    
    if (choice >= 1 && choice <= 5) {
        // Recalculate bill if readings were updated
        if (choice == 4 || choice == 5) {
            calculateBill(*it);
        }
        cout << "Customer details updated successfully!\n";
    }
    
    pressEnterToContinue();
}

void deleteCustomer() {
    clearScreen();
    cout << "=== DELETE CUSTOMER ===\n\n";
    
    if (customers.empty()) {
        cout << "No customers found!\n";
        pressEnterToContinue();
        return;
    }
    
    int id = getValidInt("Enter Customer ID to delete: ");
    
    auto it = find_if(customers.begin(), customers.end(),
                     [id](const Customer &c) { return c.customerID == id; });
    
    if (it == customers.end()) {
        cout << "Customer not found with ID: " << id << endl;
        pressEnterToContinue();
        return;
    }
    
    cout << "\nCustomer Found:\n";
    cout << "ID: " << it->customerID << ", Name: " << it->name << endl;
    
    char confirm;
    cout << "Are you sure you want to delete this customer? (y/n): ";
    cin >> confirm;
    
    if (tolower(confirm) == 'y') {
        customers.erase(it);
        cout << "Customer deleted successfully!\n";
    } else {
        cout << "Deletion cancelled.\n";
    }
    
    pressEnterToContinue();
}

void payBill() {
    clearScreen();
    cout << "=== PAY BILL ===\n\n";
    
    if (customers.empty()) {
        cout << "No customers found!\n";
        pressEnterToContinue();
        return;
    }
    
    int id = getValidInt("Enter Customer ID to pay bill: ");
    
    auto it = find_if(customers.begin(), customers.end(),
                     [id](const Customer &c) { return c.customerID == id; });
    
    if (it == customers.end()) {
        cout << "Customer not found with ID: " << id << endl;
        pressEnterToContinue();
        return;
    }
    
    if (it->isPaid) {
        cout << "Bill is already paid!\n";
        pressEnterToContinue();
        return;
    }
    
    cout << "\nCustomer: " << it->name << endl;
    cout << "Bill Amount: Rs. " << fixed << setprecision(2) << it->billAmount << endl;
    cout << "Billing Date: " << it->billingDate << endl;
    
    char confirm;
    cout << "\nConfirm payment? (y/n): ";
    cin >> confirm;
    
    if (tolower(confirm) == 'y') {
        it->isPaid = true;
        cout << "Payment recorded successfully!\n";
    } else {
        cout << "Payment cancelled.\n";
    }
    
    pressEnterToContinue();
}

void viewPaidBills() {
    clearScreen();
    cout << "=== PAID BILLS ===\n\n";
    
    bool found = false;
    double totalPaid = 0.0;
    
    cout << left << setw(10) << "ID" 
         << setw(20) << "Name" 
         << setw(15) << "Bill Date" 
         << setw(15) << "Amount" << endl;
    cout << string(60, '-') << endl;
    
    cout << fixed << setprecision(2);
    for (const auto &customer : customers) {
        if (customer.isPaid) {
            found = true;
            totalPaid += customer.billAmount;
            cout << left << setw(10) << customer.customerID
                 << setw(20) << (customer.name.length() > 18 ? customer.name.substr(0, 15) + "..." : customer.name)
                 << setw(15) << customer.billingDate
                 << setw(15) << customer.billAmount << endl;
        }
    }
    
    if (!found) {
        cout << "No paid bills found!\n";
    } else {
        cout << string(60, '-') << endl;
        cout << "Total Paid Amount: Rs. " << totalPaid << endl;
    }
    
    pressEnterToContinue();
}

void viewPendingBills() {
    clearScreen();
    cout << "=== PENDING BILLS ===\n\n";
    
    bool found = false;
    double totalPending = 0.0;
    
    cout << left << setw(10) << "ID" 
         << setw(20) << "Name" 
         << setw(15) << "Bill Date" 
         << setw(15) << "Amount" << endl;
    cout << string(60, '-') << endl;
    
    cout << fixed << setprecision(2);
    for (const auto &customer : customers) {
        if (!customer.isPaid) {
            found = true;
            totalPending += customer.billAmount;
            cout << left << setw(10) << customer.customerID
                 << setw(20) << (customer.name.length() > 18 ? customer.name.substr(0, 15) + "..." : customer.name)
                 << setw(15) << customer.billingDate
                 << setw(15) << customer.billAmount << endl;
        }
    }
    
    if (!found) {
        cout << "No pending bills found!\n";
    } else {
        cout << string(60, '-') << endl;
        cout << "Total Pending Amount: Rs. " << totalPending << endl;
    }
    
    pressEnterToContinue();
}

void updateTariff() {
    clearScreen();
    cout << "=== UPDATE TARIFF RATES ===\n\n";
    
    cout << "Current Tariff Rates:\n";
    cout << "1. Domestic Rate: Rs. " << currentTariff.domesticRate << " per unit\n";
    cout << "2. Commercial Rate: Rs. " << currentTariff.commercialRate << " per unit\n";
    cout << "3. Industrial Rate: Rs. " << currentTariff.industrialRate << " per unit\n";
    
    int choice;
    cout << "\nSelect rate to update (1-3, 0 to cancel): ";
    cin >> choice;
    
    if (choice >= 1 && choice <= 3) {
        double newRate;
        cout << "Enter new rate: Rs. ";
        cin >> newRate;
        
        if (newRate < 0) {
            cout << "Rate cannot be negative!\n";
        } else {
            switch(choice) {
                case 1:
                    currentTariff.domesticRate = newRate;
                    break;
                case 2:
                    currentTariff.commercialRate = newRate;
                    break;
                case 3:
                    currentTariff.industrialRate = newRate;
                    break;
            }
            cout << "Tariff rate updated successfully!\n";
        }
    } else if (choice != 0) {
        cout << "Invalid choice!\n";
    }
    
    pressEnterToContinue();
}

void generateReport() {
    clearScreen();
    cout << "=== SYSTEM REPORT ===\n\n";
    
    int totalCustomers = customers.size();
    int paidBills = 0;
    int pendingBills = 0;
    double totalRevenue = 0.0;
    double totalPending = 0.0;
    
    for (const auto &customer : customers) {
        if (customer.isPaid) {
            paidBills++;
            totalRevenue += customer.billAmount;
        } else {
            pendingBills++;
            totalPending += customer.billAmount;
        }
    }
    
    cout << "System Statistics:\n";
    cout << "------------------\n";
    cout << "Total Customers: " << totalCustomers << endl;
    cout << "Paid Bills: " << paidBills << endl;
    cout << "Pending Bills: " << pendingBills << endl;
    cout << fixed << setprecision(2);
    cout << "Total Revenue Collected: Rs. " << totalRevenue << endl;
    cout << "Total Pending Amount: Rs. " << totalPending << endl;
    cout << "------------------\n\n";
    
    cout << "Tariff Rates:\n";
    cout << "-------------\n";
    cout << "Domestic Rate: Rs. " << currentTariff.domesticRate << " per unit\n";
    cout << "Commercial Rate: Rs. " << currentTariff.commercialRate << " per unit\n";
    cout << "Industrial Rate: Rs. " << currentTariff.industrialRate << " per unit\n";
    
    pressEnterToContinue();
}

void saveData() {
    ofstream outFile(DATA_FILE, ios::binary);
    if (!outFile) {
        cout << "Error saving data to file!\n";
        return;
    }
    
    size_t count = customers.size();
    outFile.write(reinterpret_cast<char*>(&count), sizeof(count));
    
    for (const auto &customer : customers) {
        // Write customer ID
        outFile.write(reinterpret_cast<const char*>(&customer.customerID), sizeof(customer.customerID));
        
        // Write string lengths and strings
        size_t nameLen = customer.name.length();
        outFile.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        outFile.write(customer.name.c_str(), nameLen);
        
        size_t addrLen = customer.address.length();
        outFile.write(reinterpret_cast<const char*>(&addrLen), sizeof(addrLen));
        outFile.write(customer.address.c_str(), addrLen);
        
        size_t contactLen = customer.contact.length();
        outFile.write(reinterpret_cast<const char*>(&contactLen), sizeof(contactLen));
        outFile.write(customer.contact.c_str(), contactLen);
        
        // Write numeric data
        outFile.write(reinterpret_cast<const char*>(&customer.previousReading), sizeof(customer.previousReading));
        outFile.write(reinterpret_cast<const char*>(&customer.currentReading), sizeof(customer.currentReading));
        outFile.write(reinterpret_cast<const char*>(&customer.unitsConsumed), sizeof(customer.unitsConsumed));
        outFile.write(reinterpret_cast<const char*>(&customer.billAmount), sizeof(customer.billAmount));
        
        // Write billing date
        size_t dateLen = customer.billingDate.length();
        outFile.write(reinterpret_cast<const char*>(&dateLen), sizeof(dateLen));
        outFile.write(customer.billingDate.c_str(), dateLen);
        
        // Write payment status
        outFile.write(reinterpret_cast<const char*>(&customer.isPaid), sizeof(customer.isPaid));
    }
    
    outFile.close();
}

void loadData() {
    ifstream inFile(DATA_FILE, ios::binary);
    if (!inFile) {
        cout << "No existing data found. Starting with empty database.\n";
        return;
    }
    
    customers.clear();
    size_t count;
    inFile.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    for (size_t i = 0; i < count; ++i) {
        Customer customer;
        
        // Read customer ID
        inFile.read(reinterpret_cast<char*>(&customer.customerID), sizeof(customer.customerID));
        
        // Read name
        size_t nameLen;
        inFile.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        char* nameBuffer = new char[nameLen + 1];
        inFile.read(nameBuffer, nameLen);
        nameBuffer[nameLen] = '\0';
        customer.name = nameBuffer;
        delete[] nameBuffer;
        
        // Read address
        size_t addrLen;
        inFile.read(reinterpret_cast<char*>(&addrLen), sizeof(addrLen));
        char* addrBuffer = new char[addrLen + 1];
        inFile.read(addrBuffer, addrLen);
        addrBuffer[addrLen] = '\0';
        customer.address = addrBuffer;
        delete[] addrBuffer;
        
        // Read contact
        size_t contactLen;
        inFile.read(reinterpret_cast<char*>(&contactLen), sizeof(contactLen));
        char* contactBuffer = new char[contactLen + 1];
        inFile.read(contactBuffer, contactLen);
        contactBuffer[contactLen] = '\0';
        customer.contact = contactBuffer;
        delete[] contactBuffer;
        
        // Read numeric data
        inFile.read(reinterpret_cast<char*>(&customer.previousReading), sizeof(customer.previousReading));
        inFile.read(reinterpret_cast<char*>(&customer.currentReading), sizeof(customer.currentReading));
        inFile.read(reinterpret_cast<char*>(&customer.unitsConsumed), sizeof(customer.unitsConsumed));
        inFile.read(reinterpret_cast<char*>(&customer.billAmount), sizeof(customer.billAmount));
        
        // Read billing date
        size_t dateLen;
        inFile.read(reinterpret_cast<char*>(&dateLen), sizeof(dateLen));
        char* dateBuffer = new char[dateLen + 1];
        inFile.read(dateBuffer, dateLen);
        dateBuffer[dateLen] = '\0';
        customer.billingDate = dateBuffer;
        delete[] dateBuffer;
        
        // Read payment status
        inFile.read(reinterpret_cast<char*>(&customer.isPaid), sizeof(customer.isPaid));
        
        customers.push_back(customer);
    }
    
    inFile.close();
    cout << "Loaded " << customers.size() << " customer records.\n";
}

void saveTariff() {
    ofstream outFile(TARIFF_FILE, ios::binary);
    if (!outFile) {
        cout << "Error saving tariff data!\n";
        return;
    }
    
    outFile.write(reinterpret_cast<const char*>(&currentTariff), sizeof(currentTariff));
    outFile.close();
}

void loadTariff() {
    ifstream inFile(TARIFF_FILE, ios::binary);
    if (!inFile) {
        cout << "No tariff data found. Using default rates.\n";
        return;
    }
    
    inFile.read(reinterpret_cast<char*>(&currentTariff), sizeof(currentTariff));
    inFile.close();
}

string getCurrentDate() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    
    stringstream ss;
    ss << (localTime->tm_year + 1900) << "-"
       << setw(2) << setfill('0') << (localTime->tm_mon + 1) << "-"
       << setw(2) << setfill('0') << localTime->tm_mday;
    
    return ss.str();
}

int generateCustomerID() {
    static int lastID = 1000; // Starting ID
    if (!customers.empty()) {
        // Find the maximum ID in the existing customers
        int maxID = 0;
        for (const auto &customer : customers) {
            if (customer.customerID > maxID) {
                maxID = customer.customerID;
            }
        }
        return maxID + 1;
    }
    return ++lastID;
}

void clearScreen() {
    // Platform-specific screen clearing
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pressEnterToContinue() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

double getValidDouble(const string &prompt) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        
        if (cin.fail() || value < 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a non-negative number.\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

int getValidInt(const string &prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        
        if (cin.fail() || value < 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a non-negative integer.\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}