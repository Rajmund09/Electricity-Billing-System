# Electricity Billing System

A C++ electricity billing application with a professional web-style frontend concept for managing customers, generating bills, tracking payments, and presenting reports more elegantly.

## Features

- Customer management with add, search, update, and delete operations
- Customer type support for `Domestic`, `Commercial`, and `Industrial`
- Professional bill calculation with unit charge, fixed charge, and tax breakdown
- Payment tracking for paid and pending bills
- Summary reporting for revenue, pending dues, and average consumption
- Persistent storage using `customers.dat` and `tariff.dat`
- Improved input validation and safer binary file handling
- Responsive frontend dashboard with polished UI, animations, and interactive billing previews

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
- [index.html](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/index.html) - frontend dashboard layout
- [styles.css](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/styles.css) - visual design, responsive layout, and animations
- [script.js](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/script.js) - UI interactions and frontend billing simulation
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

## Frontend Preview

Open [index.html](/c:/Users/prabh/OneDrive/Documents/GitHub/Electricity-Billing-System/index.html) in a browser to view the modern UI concept built around the same modules as the C++ app:

- Overview metrics
- Customer directory
- Bill generation preview
- Tariff cards
- Report visualization

This frontend is currently a local UI prototype and does not directly execute the C++ binary. It is designed as the presentation layer you could later connect to the backend through an API, local service, or file-based bridge.

## Improvements Made

- Replaced global state with a dedicated application class
- Introduced clearer data modeling with `CustomerType` and bill breakdown logic
- Strengthened numeric and text input validation
- Added more reliable save/load helpers for binary persistence
- Improved menu flow, formatting, and reporting output
- Aligned the implementation with the tariff types already described by the project
- Added a frontend experience with animations, polished layout, and smoother operator interactions
