const tariffRates = {
    Domestic: 5.0,
    Commercial: 7.5,
    Industrial: 10.0
};

const state = {
    customers: [
        {
            id: 1001,
            name: "Aarav Sharma",
            address: "14 Lakeview Residency, Pune",
            contact: "+91 98765 12001",
            type: "Domestic",
            previousReading: 1850,
            currentReading: 2015,
            unitsConsumed: 165,
            billAmount: 1032.5,
            isPaid: true
        },
        {
            id: 1002,
            name: "Blue Orbit Mart",
            address: "22 Market Spine Road, Nashik",
            contact: "+91 98765 12002",
            type: "Commercial",
            previousReading: 7120,
            currentReading: 7435,
            unitsConsumed: 315,
            billAmount: 2843.5,
            isPaid: false
        },
        {
            id: 1003,
            name: "Vega Steel Works",
            address: "Sector 8 Industrial Belt, Nagpur",
            contact: "+91 98765 12003",
            type: "Industrial",
            previousReading: 12540,
            currentReading: 13010,
            unitsConsumed: 470,
            billAmount: 5605,
            isPaid: false
        }
    ],
    previewCustomerId: 1001
};

const customerTableBody = document.getElementById("customerTableBody");
const customerSearch = document.getElementById("customerSearch");
const customerForm = document.getElementById("customerForm");
const customerFormSection = document.getElementById("customerFormSection");
const billingForm = document.getElementById("billingForm");
const billingCustomer = document.getElementById("billingCustomer");
const previousReading = document.getElementById("previousReading");
const currentReading = document.getElementById("currentReading");
const previewUnits = document.getElementById("previewUnits");
const previewEnergy = document.getElementById("previewEnergy");
const previewTax = document.getElementById("previewTax");
const previewTotal = document.getElementById("previewTotal");
const previewStatus = document.getElementById("previewStatus");
const billPreview = document.getElementById("billPreview");
const tariffCards = document.getElementById("tariffCards");
const collectionChart = document.getElementById("collectionChart");
const paidCount = document.getElementById("paidCount");
const pendingCount = document.getElementById("pendingCount");
const topSegment = document.getElementById("topSegment");
const toast = document.getElementById("toast");

const metricTargets = {
    revenue: document.querySelector('[data-metric="revenue"]'),
    pending: document.querySelector('[data-metric="pending"]'),
    customers: document.querySelector('[data-metric="customers"]'),
    units: document.querySelector('[data-metric="units"]')
};

function calculateBill(previous, current, type) {
    const units = Math.max(0, current - previous);
    const energy = units * tariffRates[type];
    const fixedCharge = 50;
    const tax = (energy + fixedCharge) * 0.18;
    const total = energy + fixedCharge + tax;

    return {
        units,
        energy,
        fixedCharge,
        tax,
        total
    };
}

function formatCurrency(value) {
    return `Rs. ${value.toFixed(2)}`;
}

function nextCustomerId() {
    return Math.max(...state.customers.map((customer) => customer.id)) + 1;
}

function showToast(message) {
    toast.textContent = message;
    toast.classList.add("is-visible");
    clearTimeout(showToast.timer);
    showToast.timer = setTimeout(() => {
        toast.classList.remove("is-visible");
    }, 2600);
}

function animateValue(element, endValue, formatter) {
    const duration = 800;
    const startTime = performance.now();
    const match = String(element.textContent).replace(/[^0-9.]/g, "");
    const startValue = Number(match) || 0;

    function frame(currentTime) {
        const progress = Math.min((currentTime - startTime) / duration, 1);
        const eased = 1 - Math.pow(1 - progress, 3);
        const currentValue = startValue + (endValue - startValue) * eased;
        element.textContent = formatter(currentValue);
        if (progress < 1) {
            requestAnimationFrame(frame);
        }
    }

    requestAnimationFrame(frame);
}

function renderMetrics() {
    const revenue = state.customers
        .filter((customer) => customer.isPaid)
        .reduce((sum, customer) => sum + customer.billAmount, 0);
    const pendingBills = state.customers.filter((customer) => !customer.isPaid).length;
    const totalCustomers = state.customers.length;
    const averageUnits = totalCustomers
        ? state.customers.reduce((sum, customer) => sum + customer.unitsConsumed, 0) / totalCustomers
        : 0;

    animateValue(metricTargets.revenue, revenue, (value) => formatCurrency(value));
    animateValue(metricTargets.pending, pendingBills, (value) => `${Math.round(value)}`);
    animateValue(metricTargets.customers, totalCustomers, (value) => `${Math.round(value)}`);
    animateValue(metricTargets.units, averageUnits, (value) => value.toFixed(1));

    paidCount.textContent = state.customers.filter((customer) => customer.isPaid).length;
    pendingCount.textContent = pendingBills;

    const segmentTotals = state.customers.reduce((accumulator, customer) => {
        accumulator[customer.type] = (accumulator[customer.type] || 0) + customer.billAmount;
        return accumulator;
    }, {});

    const bestSegment = Object.entries(segmentTotals).sort((a, b) => b[1] - a[1])[0];
    topSegment.textContent = bestSegment ? bestSegment[0] : "Domestic";
}

function statusBadge(isPaid) {
    const label = isPaid ? "Paid" : "Pending";
    const className = isPaid ? "status-paid" : "status-pending";
    return `<span class="status-badge ${className}">${label}</span>`;
}

function renderCustomerTable(filter = "") {
    const query = filter.trim().toLowerCase();
    const customers = state.customers.filter((customer) => {
        return (
            customer.name.toLowerCase().includes(query) ||
            String(customer.id).includes(query) ||
            customer.type.toLowerCase().includes(query)
        );
    });

    customerTableBody.innerHTML = customers.map((customer) => `
        <tr>
            <td>${customer.id}</td>
            <td>
                <strong>${customer.name}</strong>
                <div>${customer.contact}</div>
            </td>
            <td>${customer.type}</td>
            <td>${customer.unitsConsumed.toFixed(2)}</td>
            <td>${formatCurrency(customer.billAmount)}</td>
            <td>${statusBadge(customer.isPaid)}</td>
        </tr>
    `).join("");
}

function renderBillingCustomers() {
    billingCustomer.innerHTML = state.customers.map((customer) => `
        <option value="${customer.id}">${customer.id} • ${customer.name} (${customer.type})</option>
    `).join("");

    if (!state.customers.some((customer) => customer.id === state.previewCustomerId)) {
        state.previewCustomerId = state.customers[0]?.id;
    }

    billingCustomer.value = String(state.previewCustomerId);
    syncBillingCustomer();
}

function syncBillingCustomer() {
    const selected = state.customers.find((customer) => customer.id === Number(billingCustomer.value));
    if (!selected) {
        return;
    }

    state.previewCustomerId = selected.id;
    previousReading.value = selected.currentReading.toFixed(2);
    currentReading.value = selected.currentReading.toFixed(2);
    updatePreview(selected.currentReading, true);
}

function updatePreview(nextReading, isNeutral = false) {
    const customer = state.customers.find((entry) => entry.id === state.previewCustomerId);
    if (!customer) {
        return;
    }

    const parsedReading = Number(nextReading);
    if (Number.isNaN(parsedReading) || parsedReading < customer.currentReading) {
        previewStatus.textContent = "Reading must increase";
        previewStatus.className = "status-pill status-pending";
        return;
    }

    const bill = calculateBill(customer.currentReading, parsedReading, customer.type);
    previewUnits.textContent = bill.units.toFixed(2);
    previewEnergy.textContent = formatCurrency(bill.energy);
    previewTax.textContent = formatCurrency(bill.tax);
    previewTotal.textContent = formatCurrency(bill.total);
    previewStatus.textContent = isNeutral ? "Ready for generation" : "Preview updated";
    previewStatus.className = "status-pill status-paid";

    billPreview.classList.remove("is-emphasized");
    requestAnimationFrame(() => billPreview.classList.add("is-emphasized"));
}

function renderTariffCards() {
    tariffCards.innerHTML = Object.entries(tariffRates).map(([type, rate]) => `
        <article class="tariff-card">
            <p class="eyebrow">${type} Segment</p>
            <h3>${type} Tariff</h3>
            <p>Aligned with the corresponding C++ billing category and charge formula.</p>
            <span class="tariff-rate">${formatCurrency(rate)}</span>
        </article>
    `).join("");
}

function renderChart() {
    const segments = Object.keys(tariffRates).map((type) => {
        const total = state.customers
            .filter((customer) => customer.type === type)
            .reduce((sum, customer) => sum + customer.billAmount, 0);
        return { type, total };
    });

    const maxValue = Math.max(...segments.map((segment) => segment.total), 1);

    collectionChart.innerHTML = segments.map((segment) => {
        const height = Math.max(18, (segment.total / maxValue) * 100);
        return `
            <div class="chart-bar">
                <div class="chart-bar-fill" style="height: ${height}%"></div>
                <div class="chart-label">
                    <span>${segment.type}</span>
                    <strong>${formatCurrency(segment.total)}</strong>
                </div>
            </div>
        `;
    }).join("");
}

function refreshDashboard(searchValue = customerSearch.value) {
    renderCustomerTable(searchValue);
    renderBillingCustomers();
    renderMetrics();
    renderTariffCards();
    renderChart();
}

function initNavigation() {
    const navLinks = document.querySelectorAll(".nav-link");
    navLinks.forEach((button) => {
        button.addEventListener("click", () => {
            navLinks.forEach((link) => link.classList.remove("is-active"));
            button.classList.add("is-active");
            document.getElementById(button.dataset.target)?.scrollIntoView({
                behavior: "smooth",
                block: "start"
            });
        });
    });
}

customerSearch.addEventListener("input", (event) => {
    renderCustomerTable(event.target.value);
});

billingCustomer.addEventListener("change", syncBillingCustomer);

currentReading.addEventListener("input", (event) => {
    updatePreview(event.target.value);
});

billingForm.addEventListener("submit", (event) => {
    event.preventDefault();

    const customer = state.customers.find((entry) => entry.id === Number(billingCustomer.value));
    const newReading = Number(currentReading.value);
    if (!customer || Number.isNaN(newReading) || newReading < customer.currentReading) {
        showToast("Current reading must be greater than or equal to the previous reading.");
        return;
    }

    const bill = calculateBill(customer.currentReading, newReading, customer.type);
    customer.previousReading = customer.currentReading;
    customer.currentReading = newReading;
    customer.unitsConsumed = bill.units;
    customer.billAmount = Number(bill.total.toFixed(2));
    customer.isPaid = false;

    refreshDashboard();
    updatePreview(newReading, false);
    showToast(`Bill generated for ${customer.name}. Total: ${formatCurrency(customer.billAmount)}`);
});

customerForm.addEventListener("submit", (event) => {
    event.preventDefault();

    const name = document.getElementById("customerName").value.trim();
    const address = document.getElementById("customerAddress").value.trim();
    const contact = document.getElementById("customerContact").value.trim();
    const type = document.getElementById("customerType").value;
    const previous = Number(document.getElementById("customerPrevious").value);
    const current = Number(document.getElementById("customerCurrent").value);

    if (!name || !address || !contact || Number.isNaN(previous) || Number.isNaN(current) || current < previous) {
        showToast("Please fill all fields correctly. Current reading must be greater than or equal to previous reading.");
        return;
    }

    const bill = calculateBill(previous, current, type);
    state.customers.unshift({
        id: nextCustomerId(),
        name,
        address,
        contact,
        type,
        previousReading: previous,
        currentReading: current,
        unitsConsumed: bill.units,
        billAmount: Number(bill.total.toFixed(2)),
        isPaid: false
    });

    customerForm.reset();
    refreshDashboard();
    showToast(`Customer ${name} added successfully.`);
});

document.getElementById("openQuickBill").addEventListener("click", () => {
    document.getElementById("billing").scrollIntoView({ behavior: "smooth", block: "start" });
    billingCustomer.focus();
});

document.getElementById("focusCustomers").addEventListener("click", () => {
    document.getElementById("customers").scrollIntoView({ behavior: "smooth", block: "start" });
    customerSearch.focus();
});

document.getElementById("scrollToForm").addEventListener("click", () => {
    customerFormSection.scrollIntoView({ behavior: "smooth", block: "start" });
    document.getElementById("customerName").focus();
});

initNavigation();
refreshDashboard();
