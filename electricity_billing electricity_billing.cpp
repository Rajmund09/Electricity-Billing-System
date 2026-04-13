#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{

    constexpr const char *kDataFile = "customers.dat";
    constexpr const char *kTariffFile = "tariff.dat";
    constexpr double kFixedCharge = 50.0;
    constexpr double kTaxRate = 0.18;
    constexpr int kStartingCustomerId = 1001;

    enum class CustomerType
    {
        Domestic = 1,
        Commercial = 2,
        Industrial = 3
    };

    struct Tariff
    {
        double domesticRate = 5.0;
        double commercialRate = 7.5;
        double industrialRate = 10.0;
    };

    struct BillBreakdown
    {
        double unitsConsumed = 0.0;
        double energyCharge = 0.0;
        double fixedCharge = kFixedCharge;
        double taxAmount = 0.0;
        double totalAmount = 0.0;
    };

    struct Customer
    {
        int customerId = 0;
        std::string name;
        std::string address;
        std::string contactNumber;
        CustomerType type = CustomerType::Domestic;
        double previousReading = 0.0;
        double currentReading = 0.0;
        double unitsConsumed = 0.0;
        double billAmount = 0.0;
        std::string billingDate = "N/A";
        bool isPaid = false;
    };

    std::string trim(const std::string &value)
    {
        const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch)
                                            { return std::isspace(ch) != 0; });

        if (first == value.end())
        {
            return "";
        }

        const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch)
                                           { return std::isspace(ch) != 0; })
                              .base();

        return std::string(first, last);
    }

    std::string toLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });
        return value;
    }

    std::string customerTypeToString(CustomerType type)
    {
        switch (type)
        {
        case CustomerType::Domestic:
            return "Domestic";
        case CustomerType::Commercial:
            return "Commercial";
        case CustomerType::Industrial:
            return "Industrial";
        default:
            return "Unknown";
        }
    }

    void clearScreen()
    {
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif
    }

    void pressEnterToContinue()
    {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    template <typename T>
    bool writeBinary(std::ofstream &output, const T &value)
    {
        output.write(reinterpret_cast<const char *>(&value), sizeof(T));
        return static_cast<bool>(output);
    }

    template <typename T>
    bool readBinary(std::ifstream &input, T &value)
    {
        input.read(reinterpret_cast<char *>(&value), sizeof(T));
        return static_cast<bool>(input);
    }

    bool writeString(std::ofstream &output, const std::string &value)
    {
        const std::size_t length = value.size();
        return writeBinary(output, length) && (length == 0 || static_cast<bool>(output.write(value.data(), static_cast<std::streamsize>(length))));
    }

    bool readString(std::ifstream &input, std::string &value)
    {
        std::size_t length = 0;
        if (!readBinary(input, length))
        {
            return false;
        }

        value.assign(length, '\0');
        if (length == 0)
        {
            return true;
        }

        input.read(&value[0], static_cast<std::streamsize>(length));
        return static_cast<bool>(input);
    }

    class ElectricityBillingSystem
    {
    public:
        void run()
        {
            loadTariff();
            loadCustomers();

            bool exitRequested = false;
            while (!exitRequested)
            {
                clearScreen();
                displayMenu();

                const int choice = getValidatedInt("Enter your choice: ", 1, 12);
                switch (choice)
                {
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
                    recordPayment();
                    break;
                case 8:
                    viewBillsByStatus(true);
                    break;
                case 9:
                    viewBillsByStatus(false);
                    break;
                case 10:
                    updateTariff();
                    break;
                case 11:
                    generateReport();
                    break;
                case 12:
                    saveAllData();
                    std::cout << "\nData saved successfully. Exiting...\n";
                    exitRequested = true;
                    break;
                }
            }
        }

    private:
        std::vector<Customer> customers_;
        Tariff tariff_;

        void displayMenu() const
        {
            std::cout << "=============================================\n";
            std::cout << "        ELECTRICITY BILLING SYSTEM\n";
            std::cout << "=============================================\n";
            std::cout << "1.  Add New Customer\n";
            std::cout << "2.  Generate Electricity Bill\n";
            std::cout << "3.  View All Customers\n";
            std::cout << "4.  Search Customer\n";
            std::cout << "5.  Update Customer Details\n";
            std::cout << "6.  Delete Customer\n";
            std::cout << "7.  Record Bill Payment\n";
            std::cout << "8.  View Paid Bills\n";
            std::cout << "9.  View Pending Bills\n";
            std::cout << "10. Update Tariff Rates\n";
            std::cout << "11. Generate Report\n";
            std::cout << "12. Save and Exit\n";
            std::cout << "=============================================\n";
        }

        static std::string currentDate()
        {
            const std::time_t now = std::time(nullptr);
            const std::tm *localTime = std::localtime(&now);

            std::ostringstream stream;
            stream << std::put_time(localTime, "%Y-%m-%d");
            return stream.str();
        }

        static int getValidatedInt(const std::string &prompt, int minValue, int maxValue)
        {
            while (true)
            {
                std::cout << prompt;

                int value = 0;
                std::cin >> value;
                if (std::cin.fail())
                {
                    recoverInput("Invalid input. Please enter a whole number.");
                    continue;
                }

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (value < minValue || value > maxValue)
                {
                    std::cout << "Please enter a value between " << minValue << " and " << maxValue << ".\n";
                    continue;
                }

                return value;
            }
        }

        static double getValidatedDouble(const std::string &prompt, double minValue = 0.0)
        {
            while (true)
            {
                std::cout << prompt;

                double value = 0.0;
                std::cin >> value;
                if (std::cin.fail())
                {
                    recoverInput("Invalid input. Please enter a numeric value.");
                    continue;
                }

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (value < minValue)
                {
                    std::cout << "Please enter a value greater than or equal to " << minValue << ".\n";
                    continue;
                }

                return value;
            }
        }

        static std::string getRequiredLine(const std::string &prompt)
        {
            while (true)
            {
                std::cout << prompt;
                std::string value;
                std::getline(std::cin, value);
                value = trim(value);

                if (!value.empty())
                {
                    return value;
                }

                std::cout << "This field cannot be empty.\n";
            }
        }

        static bool confirmAction(const std::string &prompt)
        {
            while (true)
            {
                std::cout << prompt << " (y/n): ";
                char answer = '\0';
                std::cin >> answer;

                if (std::cin.fail())
                {
                    recoverInput("Invalid input. Please enter y or n.");
                    continue;
                }

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                answer = static_cast<char>(std::tolower(static_cast<unsigned char>(answer)));

                if (answer == 'y')
                {
                    return true;
                }

                if (answer == 'n')
                {
                    return false;
                }

                std::cout << "Please enter y or n.\n";
            }
        }

        static void recoverInput(const std::string &message)
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << message << '\n';
        }

        int nextCustomerId() const
        {
            int maxId = kStartingCustomerId - 1;
            for (const Customer &customer : customers_)
            {
                maxId = std::max(maxId, customer.customerId);
            }
            return maxId + 1;
        }

        CustomerType chooseCustomerType(CustomerType currentType = CustomerType::Domestic) const
        {
            std::cout << "Select Customer Type:\n";
            std::cout << "1. Domestic\n";
            std::cout << "2. Commercial\n";
            std::cout << "3. Industrial\n";

            if (currentType == CustomerType::Domestic || currentType == CustomerType::Commercial || currentType == CustomerType::Industrial)
            {
                std::cout << "Current type: " << customerTypeToString(currentType) << '\n';
            }

            return static_cast<CustomerType>(getValidatedInt("Enter choice: ", 1, 3));
        }

        double rateFor(CustomerType type) const
        {
            switch (type)
            {
            case CustomerType::Domestic:
                return tariff_.domesticRate;
            case CustomerType::Commercial:
                return tariff_.commercialRate;
            case CustomerType::Industrial:
                return tariff_.industrialRate;
            default:
                return tariff_.domesticRate;
            }
        }

        BillBreakdown buildBillBreakdown(const Customer &customer) const
        {
            BillBreakdown breakdown;
            breakdown.unitsConsumed = customer.currentReading - customer.previousReading;
            breakdown.energyCharge = breakdown.unitsConsumed * rateFor(customer.type);
            const double subtotal = breakdown.energyCharge + breakdown.fixedCharge;
            breakdown.taxAmount = subtotal * kTaxRate;
            breakdown.totalAmount = subtotal + breakdown.taxAmount;
            return breakdown;
        }

        void recalculateBill(Customer &customer) const
        {
            const BillBreakdown breakdown = buildBillBreakdown(customer);
            customer.unitsConsumed = breakdown.unitsConsumed;
            customer.billAmount = breakdown.totalAmount;
            customer.billingDate = currentDate();
            customer.isPaid = false;
        }

        Customer *findCustomerById(int customerId)
        {
            auto iterator = std::find_if(customers_.begin(), customers_.end(), [customerId](const Customer &customer)
                                         { return customer.customerId == customerId; });
            return iterator == customers_.end() ? nullptr : &(*iterator);
        }

        const Customer *findCustomerById(int customerId) const
        {
            auto iterator = std::find_if(customers_.begin(), customers_.end(), [customerId](const Customer &customer)
                                         { return customer.customerId == customerId; });
            return iterator == customers_.end() ? nullptr : &(*iterator);
        }

        void addCustomer()
        {
            clearScreen();
            std::cout << "=== ADD NEW CUSTOMER ===\n\n";

            Customer customer;
            customer.customerId = nextCustomerId();
            customer.name = getRequiredLine("Enter Customer Name: ");
            customer.address = getRequiredLine("Enter Address: ");
            customer.contactNumber = getRequiredLine("Enter Contact Number: ");
            customer.type = chooseCustomerType();
            customer.previousReading = getValidatedDouble("Enter Previous Meter Reading: ", 0.0);
            customer.currentReading = getValidatedDouble("Enter Current Meter Reading: ", customer.previousReading);
            recalculateBill(customer);

            customers_.push_back(customer);
            saveAllData();

            std::cout << "\nCustomer added successfully.\n";
            printCustomerSummary(customer);
            pressEnterToContinue();
        }

        void generateBill()
        {
            clearScreen();
            std::cout << "=== GENERATE ELECTRICITY BILL ===\n\n";

            if (customers_.empty())
            {
                std::cout << "No customers found. Add a customer first.\n";
                pressEnterToContinue();
                return;
            }

            const int customerId = getValidatedInt("Enter Customer ID: ", 1, std::numeric_limits<int>::max());
            Customer *customer = findCustomerById(customerId);
            if (customer == nullptr)
            {
                std::cout << "Customer not found with ID: " << customerId << '\n';
                pressEnterToContinue();
                return;
            }

            std::cout << "\nGenerating bill for " << customer->name << " (" << customerTypeToString(customer->type) << ")\n";
            customer->previousReading = customer->currentReading;
            customer->currentReading = getValidatedDouble("Enter New Meter Reading: ", customer->previousReading);
            recalculateBill(*customer);
            saveAllData();

            clearScreen();
            printBill(*customer);
            pressEnterToContinue();
        }

        void viewAllCustomers() const
        {
            clearScreen();
            std::cout << "=== ALL CUSTOMERS ===\n\n";

            if (customers_.empty())
            {
                std::cout << "No customers found.\n";
                pressEnterToContinue();
                return;
            }

            std::cout << std::left
                      << std::setw(8) << "ID"
                      << std::setw(22) << "Name"
                      << std::setw(14) << "Type"
                      << std::setw(14) << "Units"
                      << std::setw(14) << "Amount"
                      << std::setw(10) << "Status" << '\n';
            std::cout << std::string(82, '-') << '\n';

            std::cout << std::fixed << std::setprecision(2);
            for (const Customer &customer : customers_)
            {
                std::cout << std::left
                          << std::setw(8) << customer.customerId
                          << std::setw(22) << shorten(customer.name, 20)
                          << std::setw(14) << customerTypeToString(customer.type)
                          << std::setw(14) << customer.unitsConsumed
                          << std::setw(14) << customer.billAmount
                          << std::setw(10) << (customer.isPaid ? "PAID" : "PENDING") << '\n';
            }

            pressEnterToContinue();
        }

        void searchCustomer() const
        {
            clearScreen();
            std::cout << "=== SEARCH CUSTOMER ===\n\n";

            if (customers_.empty())
            {
                std::cout << "No customers found.\n";
                pressEnterToContinue();
                return;
            }

            std::cout << "1. Search by Customer ID\n";
            std::cout << "2. Search by Name\n";
            const int choice = getValidatedInt("Enter choice: ", 1, 2);

            if (choice == 1)
            {
                const int customerId = getValidatedInt("Enter Customer ID: ", 1, std::numeric_limits<int>::max());
                const Customer *customer = findCustomerById(customerId);
                if (customer == nullptr)
                {
                    std::cout << "Customer not found with ID: " << customerId << '\n';
                }
                else
                {
                    printCustomerDetails(*customer);
                }
                pressEnterToContinue();
                return;
            }

            const std::string keyword = toLower(getRequiredLine("Enter full or partial customer name: "));
            bool found = false;

            std::cout << '\n';
            for (const Customer &customer : customers_)
            {
                if (toLower(customer.name).find(keyword) != std::string::npos)
                {
                    found = true;
                    printCustomerSummary(customer);
                    std::cout << '\n';
                }
            }

            if (!found)
            {
                std::cout << "No customers matched your search.\n";
            }

            pressEnterToContinue();
        }

        void updateCustomer()
        {
            clearScreen();
            std::cout << "=== UPDATE CUSTOMER DETAILS ===\n\n";

            if (customers_.empty())
            {
                std::cout << "No customers found.\n";
                pressEnterToContinue();
                return;
            }

            const int customerId = getValidatedInt("Enter Customer ID to update: ", 1, std::numeric_limits<int>::max());
            Customer *customer = findCustomerById(customerId);
            if (customer == nullptr)
            {
                std::cout << "Customer not found with ID: " << customerId << '\n';
                pressEnterToContinue();
                return;
            }

            printCustomerDetails(*customer);
            std::cout << "\n1. Name\n";
            std::cout << "2. Address\n";
            std::cout << "3. Contact Number\n";
            std::cout << "4. Customer Type\n";
            std::cout << "5. Previous Reading\n";
            std::cout << "6. Current Reading\n";
            std::cout << "7. Cancel\n";

            const int choice = getValidatedInt("Select field to update: ", 1, 7);
            bool billNeedsRecalculation = false;

            switch (choice)
            {
            case 1:
                customer->name = getRequiredLine("Enter new Name: ");
                break;
            case 2:
                customer->address = getRequiredLine("Enter new Address: ");
                break;
            case 3:
                customer->contactNumber = getRequiredLine("Enter new Contact Number: ");
                break;
            case 4:
                customer->type = chooseCustomerType(customer->type);
                billNeedsRecalculation = true;
                break;
            case 5:
                customer->previousReading = getValidatedDouble("Enter new Previous Reading: ", 0.0);
                if (customer->currentReading < customer->previousReading)
                {
                    customer->currentReading = customer->previousReading;
                }
                billNeedsRecalculation = true;
                break;
            case 6:
                customer->currentReading = getValidatedDouble("Enter new Current Reading: ", customer->previousReading);
                billNeedsRecalculation = true;
                break;
            case 7:
                std::cout << "Update cancelled.\n";
                pressEnterToContinue();
                return;
            }

            if (billNeedsRecalculation)
            {
                recalculateBill(*customer);
            }

            saveAllData();
            std::cout << "Customer updated successfully.\n";
            pressEnterToContinue();
        }

        void deleteCustomer()
        {
            clearScreen();
            std::cout << "=== DELETE CUSTOMER ===\n\n";

            if (customers_.empty())
            {
                std::cout << "No customers found.\n";
                pressEnterToContinue();
                return;
            }

            const int customerId = getValidatedInt("Enter Customer ID to delete: ", 1, std::numeric_limits<int>::max());
            auto iterator = std::find_if(customers_.begin(), customers_.end(), [customerId](const Customer &customer)
                                         { return customer.customerId == customerId; });

            if (iterator == customers_.end())
            {
                std::cout << "Customer not found with ID: " << customerId << '\n';
                pressEnterToContinue();
                return;
            }

            std::cout << "Customer: " << iterator->name << " (" << iterator->customerId << ")\n";
            if (confirmAction("Delete this customer"))
            {
                customers_.erase(iterator);
                saveAllData();
                std::cout << "Customer deleted successfully.\n";
            }
            else
            {
                std::cout << "Deletion cancelled.\n";
            }

            pressEnterToContinue();
        }

        void recordPayment()
        {
            clearScreen();
            std::cout << "=== RECORD BILL PAYMENT ===\n\n";

            if (customers_.empty())
            {
                std::cout << "No customers found.\n";
                pressEnterToContinue();
                return;
            }

            const int customerId = getValidatedInt("Enter Customer ID: ", 1, std::numeric_limits<int>::max());
            Customer *customer = findCustomerById(customerId);
            if (customer == nullptr)
            {
                std::cout << "Customer not found with ID: " << customerId << '\n';
                pressEnterToContinue();
                return;
            }

            if (customer->billAmount <= 0.0)
            {
                std::cout << "No bill has been generated for this customer yet.\n";
                pressEnterToContinue();
                return;
            }

            if (customer->isPaid)
            {
                std::cout << "This bill is already marked as paid.\n";
                pressEnterToContinue();
                return;
            }

            printCustomerSummary(*customer);
            if (confirmAction("Confirm payment"))
            {
                customer->isPaid = true;
                saveAllData();
                std::cout << "Payment recorded successfully.\n";
            }
            else
            {
                std::cout << "Payment cancelled.\n";
            }

            pressEnterToContinue();
        }

        void viewBillsByStatus(bool paid) const
        {
            clearScreen();
            std::cout << (paid ? "=== PAID BILLS ===\n\n" : "=== PENDING BILLS ===\n\n");

            bool found = false;
            double totalAmount = 0.0;

            std::cout << std::left
                      << std::setw(8) << "ID"
                      << std::setw(22) << "Name"
                      << std::setw(14) << "Type"
                      << std::setw(14) << "Bill Date"
                      << std::setw(14) << "Amount" << '\n';
            std::cout << std::string(72, '-') << '\n';

            std::cout << std::fixed << std::setprecision(2);
            for (const Customer &customer : customers_)
            {
                if (customer.isPaid == paid)
                {
                    found = true;
                    totalAmount += customer.billAmount;
                    std::cout << std::left
                              << std::setw(8) << customer.customerId
                              << std::setw(22) << shorten(customer.name, 20)
                              << std::setw(14) << customerTypeToString(customer.type)
                              << std::setw(14) << customer.billingDate
                              << std::setw(14) << customer.billAmount << '\n';
                }
            }

            if (!found)
            {
                std::cout << (paid ? "No paid bills found.\n" : "No pending bills found.\n");
            }
            else
            {
                std::cout << std::string(72, '-') << '\n';
                std::cout << "Total " << (paid ? "Paid" : "Pending") << " Amount: Rs. " << totalAmount << '\n';
            }

            pressEnterToContinue();
        }

        void updateTariff()
        {
            clearScreen();
            std::cout << "=== UPDATE TARIFF RATES ===\n\n";

            printTariffTable();
            std::cout << "\n1. Update Domestic Rate\n";
            std::cout << "2. Update Commercial Rate\n";
            std::cout << "3. Update Industrial Rate\n";
            std::cout << "4. Cancel\n";

            const int choice = getValidatedInt("Enter choice: ", 1, 4);
            if (choice == 4)
            {
                std::cout << "Tariff update cancelled.\n";
                pressEnterToContinue();
                return;
            }

            const double newRate = getValidatedDouble("Enter new rate (Rs. per unit): ", 0.0);
            switch (choice)
            {
            case 1:
                tariff_.domesticRate = newRate;
                break;
            case 2:
                tariff_.commercialRate = newRate;
                break;
            case 3:
                tariff_.industrialRate = newRate;
                break;
            }

            saveAllData();
            std::cout << "Tariff updated successfully.\n";
            pressEnterToContinue();
        }

        void generateReport() const
        {
            clearScreen();
            std::cout << "=== SYSTEM REPORT ===\n\n";

            int paidCount = 0;
            int pendingCount = 0;
            double totalRevenue = 0.0;
            double totalPending = 0.0;
            double totalUnits = 0.0;

            for (const Customer &customer : customers_)
            {
                totalUnits += customer.unitsConsumed;
                if (customer.isPaid)
                {
                    ++paidCount;
                    totalRevenue += customer.billAmount;
                }
                else
                {
                    ++pendingCount;
                    totalPending += customer.billAmount;
                }
            }

            const double averageUnits = customers_.empty() ? 0.0 : totalUnits / customers_.size();

            std::cout << "Customer Summary\n";
            std::cout << "----------------\n";
            std::cout << "Total Customers        : " << customers_.size() << '\n';
            std::cout << "Paid Bills             : " << paidCount << '\n';
            std::cout << "Pending Bills          : " << pendingCount << '\n';
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Total Revenue Collected: Rs. " << totalRevenue << '\n';
            std::cout << "Total Pending Amount   : Rs. " << totalPending << '\n';
            std::cout << "Average Units Consumed : " << averageUnits << '\n';
            std::cout << '\n';

            std::cout << "Current Tariff\n";
            std::cout << "--------------\n";
            printTariffTable();

            pressEnterToContinue();
        }

        void printTariffTable() const
        {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Domestic   : Rs. " << tariff_.domesticRate << " per unit\n";
            std::cout << "Commercial : Rs. " << tariff_.commercialRate << " per unit\n";
            std::cout << "Industrial : Rs. " << tariff_.industrialRate << " per unit\n";
        }

        static std::string shorten(const std::string &value, std::size_t maxLength)
        {
            if (value.size() <= maxLength)
            {
                return value;
            }
            if (maxLength <= 3)
            {
                return value.substr(0, maxLength);
            }
            return value.substr(0, maxLength - 3) + "...";
        }

        void printBill(const Customer &customer) const
        {
            const BillBreakdown breakdown = buildBillBreakdown(customer);

            std::cout << "=============================================\n";
            std::cout << "              ELECTRICITY BILL\n";
            std::cout << "=============================================\n";
            std::cout << "Bill Date     : " << customer.billingDate << '\n';
            std::cout << "Customer ID   : " << customer.customerId << '\n';
            std::cout << "Name          : " << customer.name << '\n';
            std::cout << "Type          : " << customerTypeToString(customer.type) << '\n';
            std::cout << "Address       : " << customer.address << '\n';
            std::cout << "Contact       : " << customer.contactNumber << '\n';
            std::cout << "---------------------------------------------\n";
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Previous Read : " << customer.previousReading << " units\n";
            std::cout << "Current Read  : " << customer.currentReading << " units\n";
            std::cout << "Units Used    : " << breakdown.unitsConsumed << " units\n";
            std::cout << "Rate Applied  : Rs. " << rateFor(customer.type) << " per unit\n";
            std::cout << "Energy Charge : Rs. " << breakdown.energyCharge << '\n';
            std::cout << "Fixed Charge  : Rs. " << breakdown.fixedCharge << '\n';
            std::cout << "Tax (18%)     : Rs. " << breakdown.taxAmount << '\n';
            std::cout << "---------------------------------------------\n";
            std::cout << "Total Amount  : Rs. " << breakdown.totalAmount << '\n';
            std::cout << "Status        : " << (customer.isPaid ? "PAID" : "PENDING") << '\n';
            std::cout << "=============================================\n";
        }

        void printCustomerSummary(const Customer &customer) const
        {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Customer ID : " << customer.customerId << '\n';
            std::cout << "Name        : " << customer.name << '\n';
            std::cout << "Type        : " << customerTypeToString(customer.type) << '\n';
            std::cout << "Contact     : " << customer.contactNumber << '\n';
            std::cout << "Bill Amount : Rs. " << customer.billAmount << '\n';
            std::cout << "Status      : " << (customer.isPaid ? "PAID" : "PENDING") << '\n';
        }

        void printCustomerDetails(const Customer &customer) const
        {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Customer ID      : " << customer.customerId << '\n';
            std::cout << "Name             : " << customer.name << '\n';
            std::cout << "Address          : " << customer.address << '\n';
            std::cout << "Contact Number   : " << customer.contactNumber << '\n';
            std::cout << "Customer Type    : " << customerTypeToString(customer.type) << '\n';
            std::cout << "Previous Reading : " << customer.previousReading << '\n';
            std::cout << "Current Reading  : " << customer.currentReading << '\n';
            std::cout << "Units Consumed   : " << customer.unitsConsumed << '\n';
            std::cout << "Bill Amount      : Rs. " << customer.billAmount << '\n';
            std::cout << "Billing Date     : " << customer.billingDate << '\n';
            std::cout << "Payment Status   : " << (customer.isPaid ? "PAID" : "PENDING") << '\n';
        }

        void saveAllData() const
        {
            saveCustomers();
            saveTariff();
        }

        void saveCustomers() const
        {
            std::ofstream output(kDataFile, std::ios::binary);
            if (!output)
            {
                std::cout << "Error: unable to save customer data.\n";
                return;
            }

            const std::size_t count = customers_.size();
            if (!writeBinary(output, count))
            {
                std::cout << "Error: failed to write customer record count.\n";
                return;
            }

            for (const Customer &customer : customers_)
            {
                const int customerTypeValue = static_cast<int>(customer.type);

                if (!writeBinary(output, customer.customerId) ||
                    !writeString(output, customer.name) ||
                    !writeString(output, customer.address) ||
                    !writeString(output, customer.contactNumber) ||
                    !writeBinary(output, customerTypeValue) ||
                    !writeBinary(output, customer.previousReading) ||
                    !writeBinary(output, customer.currentReading) ||
                    !writeBinary(output, customer.unitsConsumed) ||
                    !writeBinary(output, customer.billAmount) ||
                    !writeString(output, customer.billingDate) ||
                    !writeBinary(output, customer.isPaid))
                {
                    std::cout << "Error: failed while saving customer data.\n";
                    return;
                }
            }
        }

        void loadCustomers()
        {
            std::ifstream input(kDataFile, std::ios::binary);
            if (!input)
            {
                return;
            }

            std::size_t count = 0;
            if (!readBinary(input, count))
            {
                std::cout << "Warning: customer data file is unreadable. Starting with an empty database.\n";
                return;
            }

            std::vector<Customer> loadedCustomers;
            loadedCustomers.reserve(count);

            for (std::size_t index = 0; index < count; ++index)
            {
                Customer customer;
                int customerTypeValue = static_cast<int>(CustomerType::Domestic);

                if (!readBinary(input, customer.customerId) ||
                    !readString(input, customer.name) ||
                    !readString(input, customer.address) ||
                    !readString(input, customer.contactNumber) ||
                    !readBinary(input, customerTypeValue) ||
                    !readBinary(input, customer.previousReading) ||
                    !readBinary(input, customer.currentReading) ||
                    !readBinary(input, customer.unitsConsumed) ||
                    !readBinary(input, customer.billAmount) ||
                    !readString(input, customer.billingDate) ||
                    !readBinary(input, customer.isPaid))
                {
                    std::cout << "Warning: customer data file appears corrupted. Loaded records were discarded.\n";
                    return;
                }

                if (customerTypeValue < static_cast<int>(CustomerType::Domestic) ||
                    customerTypeValue > static_cast<int>(CustomerType::Industrial))
                {
                    customerTypeValue = static_cast<int>(CustomerType::Domestic);
                }

                customer.type = static_cast<CustomerType>(customerTypeValue);
                loadedCustomers.push_back(customer);
            }

            customers_ = std::move(loadedCustomers);
        }

        void saveTariff() const
        {
            std::ofstream output(kTariffFile, std::ios::binary);
            if (!output)
            {
                std::cout << "Error: unable to save tariff data.\n";
                return;
            }

            if (!writeBinary(output, tariff_.domesticRate) ||
                !writeBinary(output, tariff_.commercialRate) ||
                !writeBinary(output, tariff_.industrialRate))
            {
                std::cout << "Error: failed while saving tariff data.\n";
            }
        }

        void loadTariff()
        {
            std::ifstream input(kTariffFile, std::ios::binary);
            if (!input)
            {
                return;
            }

            Tariff loadedTariff;
            if (!readBinary(input, loadedTariff.domesticRate) ||
                !readBinary(input, loadedTariff.commercialRate) ||
                !readBinary(input, loadedTariff.industrialRate))
            {
                std::cout << "Warning: tariff file is unreadable. Default rates will be used.\n";
                return;
            }

            tariff_ = loadedTariff;
        }
    };

} // namespace

int main()
{
    ElectricityBillingSystem app;
    app.run();
    return 0;
}
