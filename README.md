# ⚡ Electricity Billing System

A robust, console-based management application built in C++ to automate electricity billing processes. This system handles the entire lifecycle of utility management, from customer onboarding and meter reading to tax calculation and payment tracking.

## 🌟 Key Features

### 👤 Customer Management
* **Record Keeping**: Maintain detailed profiles including unique Customer IDs, addresses, and contact information.
* **Dynamic Search**: Find records instantly using either the Customer ID or by searching for partial name matches.
* **Full CRUD Support**: Add new users, update existing details (like address or meter readings), or remove accounts from the database.

### 💰 Automated Billing Engine
* **Consumption Logic**: Automatically calculates units consumed by subtracting previous readings from current meter entries.
* **Flexible Tariffs**: Supports tiered pricing for Domestic, Commercial, and Industrial categories.
* **Tax & Fees**: Includes a built-in calculation for a fixed monthly charge (Rs. 50) and a standard 18% tax rate.

### 📊 Financial Tracking & Reporting
* **Payment Status**: Track "Paid" and "Pending" bills to manage accounts receivable.
* **System Reports**: Generate a high-level summary showing total customers, total revenue collected, and total outstanding debt.

### 💾 Data Persistence
* **Binary Storage**: Uses `customers.dat` and `tariff.dat` to ensure all data is saved permanently.
* **Auto-Load**: The system automatically retrieves your database on startup so you never lose progress.

## 🛠️ Technical Stack
* **Language**: C++
* **Paradigm**: Procedural with Structure-based data modeling
* **Libraries**: 
    * `iostream` & `fstream` for I/O and File Handling
    * `vector` & `algorithm` for data management and searching
    * `iomanip` for formatted table displays

## 🚀 Getting Started

### Prerequisites
* A C++ compiler (GCC/G++, Clang, or MSVC).
* Windows or Linux environment (the code includes platform-specific screen clearing).

### Installation & Execution
1. **Clone the repository**:
   ```bash
   git clone [https://github.com/Rajmund09/Electricity-Billing-System.git](https://github.com/Rajmund09/Electricity-Billing-System.git)
   
**Compile the code:**
   g++ electricity_billing.cpp -o BillingSystem
**Run the application:**
   ./BillingSystem