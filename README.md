# Electricity Billing System — Industrial Edition

A premium, minimal, and "solid" electricity billing interface powered by a C++ backend. This system is designed for high-performance customer management, precise bill generation, and real-time revenue tracking with an industrial "billing machine" aesthetic.

## Industrial Design System

The application features a modern **Industrial Design System** focused on technical precision and high legibility:

- **High-Contrast Dark Theme**: Deep charcoal backgrounds (`#0a0a0c`) with high-visibility amber accents.
- **Technical Typography**: 
  - **Space Grotesk**: For bold, architectural headers.
  - **JetBrains Mono**: For precise numerical data and "digital readout" metrics.
- **CRT Interface**: A subtle scanline overlay and bloom effect that reinforces the "billing machine" feel.
- **Sharp Brutalism**: Replaced soft shadows and rounded corners with solid borders and sharp, mechanical transitions.

## What Is Connected Now

The frontend is no longer a static mockup. `backend_server.cpp` runs a C++ HTTP service, serves only the approved frontend files, and exposes JSON API endpoints used by the UI.

The browser calls the C++ backend for:

- Loading customers, tariffs, and reports
- Creating customer accounts
- Generating final bills
- Marking bills paid or pending
- Deleting customer records
- Updating tariff rates and recalculating bills

## Build

Build the connected C++ web backend:

```powershell
g++ backend_server.cpp -std=c++17 -Wall -Wextra -pedantic -o BillingBackend.exe -lws2_32
```

The original console program is still available:

```powershell
g++ "electricity_billing electricity_billing.cpp" -std=c++17 -Wall -Wextra -pedantic -o BillingSystem.exe
```

## Run The Connected App

Start the C++ backend:

```powershell
.\BillingBackend.exe
```

Then open this URL in your browser:

```text
http://localhost:8080
```

Keep the terminal window open while using the app. Press `Ctrl+C` to stop the backend.

To run on a custom port:

```powershell
$env:PORT="8080"
.\BillingBackend.exe
```

## Security Notes

- The UI no longer displays internal server addresses.
- Static serving is restricted to `index.html`, `styles.css`, and `script.js`.
- Project files, source files, executables, and data files are not served as public frontend assets.
- Browser security headers are sent by the C++ service.
- Runtime data files are ignored by Git.

## Making A Live Link

This app needs a hosting provider that can run a native C++ server process. Static hosts such as GitHub Pages cannot run this backend by themselves.

Good live options:

- A VPS such as Render, Railway, Fly.io, DigitalOcean, or an AWS/Azure/GCP virtual machine
- A Docker deployment that compiles and runs `backend_server.cpp`
- A reverse proxy such as Nginx in front of `BillingBackend.exe`

For production, place HTTPS in front of the service and use the provider's assigned `PORT` environment variable.

## API Endpoints

- `GET /api/state`
- `GET /api/customers`
- `GET /api/tariffs`
- `GET /api/report`
- `POST /api/customers`
- `POST /api/bills/generate`
- `POST /api/payments`
- `POST /api/tariffs`
- `DELETE /api/customers/{id}`

## Data Files

The connected web backend stores its local data in:

- `customers_api.dat`
- `tariffs_api.dat`

The original console backend still uses its own older files:

- `customers.dat`
- `tariff.dat`

## Billing Logic

Each generated bill includes:

- Energy charge based on customer type tariff
- Fixed charge of `Rs. 50`
- Tax of `18%`

Default tariff rates:

- Domestic: `Rs. 5.00` per unit
- Commercial: `Rs. 7.50` per unit
- Industrial: `Rs. 10.00` per unit

## Project Structure

- `backend_server.cpp` - connected C++ HTTP backend and API server
- `electricity_billing electricity_billing.cpp` - original console application source
- `index.html` - connected bill generator UI
- `styles.css` - billing-console visual design
- `script.js` - frontend API integration and UI behavior
