# Electricity Billing System

A console-based C++ application for managing electricity customers, generating bills, tracking payments, and maintaining tariff rates with persistent binary storage.

## Features

- Customer management with add, search, update, and delete operations
- Customer type support for `Domestic`, `Commercial`, and `Industrial`
- Professional bill calculation with unit charge, fixed charge, and tax breakdown
- Payment tracking for paid and pending bills
- Summary reporting for revenue, pending dues, and average consumption
- Persistent storage using `customers.dat` and `tariff.dat`
- Improved input validation and safer binary file handling

## Billing Logic

Each generated bill includes:

- Energy charge based on customer type tariff
- Fixed charge of `Rs. 50`
- Tax of `18%`

Supported default tariff rates:

- Domestic: `Rs. 5.00` per unit
- Commercial: `Rs. 7.50` per unit
- Industrial: `Rs. 10.00` per unit

## Project Structure

- [electricity_billing electricity_billing.cpp](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/electricity_billing%20electricity_billing.cpp) - main application source
- [customers.dat](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/customers.dat) - saved customer records after first run
- [tariff.dat](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/tariff.dat) - saved tariff data after first run

## Build

Using `g++`:

```bash
g++ "electricity_billing electricity_billing.cpp" -std=c++17 -Wall -Wextra -pedantic -o BillingSystem
```

On Windows with MinGW:

```bash
g++ "electricity_billing electricity_billing.cpp" -std=c++17 -Wall -Wextra -pedantic -o BillingSystem.exe
```

## Run

Linux or macOS:

```bash
./BillingSystem
```

Windows:

```powershell
.\BillingSystem.exe
```

## Improvements Made

- Replaced global state with a dedicated application class
- Introduced clearer data modeling with `CustomerType` and bill breakdown logic
- Strengthened numeric and text input validation
- Added more reliable save/load helpers for binary persistence
- Improved menu flow, formatting, and reporting output
- Aligned the implementation with the tariff types already described by the project
