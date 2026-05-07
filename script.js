if (window.location.protocol === "file:") {
    window.location.replace("http://localhost:8080/");
}

const api = {
    state: "/api/state",
    customers: "/api/customers",
    generateBill: "/api/bills/generate",
    payments: "/api/payments",
    tariffs: "/api/tariffs"
};

const state = {
    customers: [],
    tariffs: {
        Domestic: 5,
        Commercial: 7.5,
        Industrial: 10,
        fixedCharge: 50,
        taxRate: 0.18
    },
    report: {},
    previewCustomerId: null
};

const $ = (id) => document.getElementById(id);

const elements = {
    customerTableBody: $("customerTableBody"),
    customerSearch: $("customerSearch"),
    customerResultCount: $("customerResultCount"),
    customerForm: $("customerForm"),
    billingForm: $("billingForm"),
    billingCustomer: $("billingCustomer"),
    previousReading: $("previousReading"),
    currentReading: $("currentReading"),
    selectedCustomerName: $("selectedCustomerName"),
    selectedCustomerType: $("selectedCustomerType"),
    selectedCustomerStatus: $("selectedCustomerStatus"),
    selectedCustomerBill: $("selectedCustomerBill"),
    previewUnits: $("previewUnits"),
    previewEnergy: $("previewEnergy"),
    previewFixed: $("previewFixed"),
    previewTax: $("previewTax"),
    previewTotal: $("previewTotal"),
    previewStatus: $("previewStatus"),
    billPreview: $("billPreview"),
    tariffCards: $("tariffCards"),
    tariffForm: $("tariffForm"),
    collectionChart: $("collectionChart"),
    paidCount: $("paidCount"),
    pendingCount: $("pendingCount"),
    pendingAmount: $("pendingAmount"),
    topSegment: $("topSegment"),
    backendStatus: $("backendStatus"),
    toast: $("toast")
};

const metricTargets = {
    revenue: document.querySelector('[data-metric="revenue"]'),
    pending: document.querySelector('[data-metric="pending"]'),
    customers: document.querySelector('[data-metric="customers"]'),
    units: document.querySelector('[data-metric="units"]')
};

function formatCurrency(value) {
    return `RS. ${Number(value || 0).toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`;
}

function setBackendStatus(mode, text) {
    elements.backendStatus.textContent = text;
    elements.backendStatus.dataset.mode = mode;
}

function showToast(message, type = "info") {
    elements.toast.textContent = message;
    elements.toast.dataset.type = type;
    elements.toast.classList.add("is-visible");
    clearTimeout(showToast.timer);
    showToast.timer = setTimeout(() => {
        elements.toast.classList.remove("is-visible");
    }, 3200);
}

function escapeHtml(value) {
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

async function requestJson(url, options = {}) {
    const response = await fetch(url, {
        headers: { "Content-Type": "application/json" },
        cache: "no-store",
        ...options
    });

    const data = await response.json().catch(() => ({}));
    if (!response.ok) {
        throw new Error(data.error || "Request failed.");
    }
    return data;
}

function applyState(nextState) {
    state.customers = Array.isArray(nextState.customers) ? nextState.customers : state.customers;
    state.tariffs = nextState.tariffs || state.tariffs;
    state.report = nextState.report || state.report;

    if (!state.customers.some((customer) => customer.id === state.previewCustomerId)) {
        state.previewCustomerId = state.customers[0]?.id ?? null;
    }

    renderAll();
}

async function loadState() {
    try {
        setBackendStatus("syncing", "Syncing records...");
        const data = await requestJson(api.state);
        applyState(data);
        setBackendStatus("online", "Records synced");
    } catch (error) {
        setBackendStatus("offline", "Service unavailable");
        showToast("Billing service is not available. Run .\\BillingBackend.exe and refresh.", "error");
    }
}

function calculatePreview(previous, current, type) {
    const units = Math.max(0, current - previous);
    const fixed = Number(state.tariffs.fixedCharge || 50);
    const taxRate = Number(state.tariffs.taxRate || 0.18);
    const energy = units * Number(state.tariffs[type] || 0);
    const tax = (energy + fixed) * taxRate;
    return { units, energy, fixed, tax, total: energy + fixed + tax };
}

function renderMetrics() {
    const report = state.report || {};
    metricTargets.revenue.textContent = formatCurrency(report.revenue || 0);
    metricTargets.pending.textContent = report.pendingCount || 0;
    metricTargets.customers.textContent = report.customerCount || 0;
    metricTargets.units.textContent = Number(report.averageUnits || 0).toFixed(1);
    elements.paidCount.textContent = report.paidCount || 0;
    elements.pendingCount.textContent = report.pendingCount || 0;
    elements.pendingAmount.textContent = formatCurrency(report.pendingAmount || 0);
    elements.topSegment.textContent = report.topSegment || "Domestic";
}

function renderCustomerTable() {
    const query = elements.customerSearch.value.trim().toLowerCase();
    const filtered = state.customers.filter((customer) => (
        customer.name.toLowerCase().includes(query) ||
        customer.address.toLowerCase().includes(query) ||
        customer.contact.toLowerCase().includes(query) ||
        customer.type.toLowerCase().includes(query) ||
        String(customer.id).includes(query)
    ));

    elements.customerResultCount.textContent = `${filtered.length} of ${state.customers.length} records shown`;

    if (!filtered.length) {
        elements.customerTableBody.innerHTML = `<tr><td class="empty-cell" colspan="7">No customer records found.</td></tr>`;
        return;
    }

    elements.customerTableBody.innerHTML = filtered.map((customer) => `
        <tr>
            <td><strong class="eyebrow">${customer.id}</strong></td>
            <td>
                <strong style="font-family: var(--font-heading);">${escapeHtml(customer.name)}</strong>
                <div class="eyebrow" style="font-size: 0.65rem; margin-top: 4px;">${escapeHtml(customer.address)}</div>
                <small class="eyebrow" style="opacity: 0.5;">${escapeHtml(customer.contact)}</small>
            </td>
            <td><span class="eyebrow">${escapeHtml(customer.type)}</span></td>
            <td><span style="font-family: var(--font-mono);">${Number(customer.currentReading).toFixed(2)}</span></td>
            <td><span style="font-family: var(--font-mono);">${Number(customer.unitsConsumed).toFixed(2)}</span></td>
            <td>
                <strong style="font-family: var(--font-mono); color: var(--accent);">${formatCurrency(customer.billAmount)}</strong>
                <div><span class="status-pill ${customer.isPaid ? "status-paid" : "status-pending"}">${customer.isPaid ? "PAID" : "PENDING"}</span></div>
            </td>
            <td class="row-actions">
                <button class="icon-button ${customer.isPaid ? "muted" : ""}" data-action="toggle-paid" data-id="${customer.id}">
                    ${customer.isPaid ? "Mark Due" : "Mark Paid"}
                </button>
                <button class="icon-button danger" data-action="delete" data-id="${customer.id}">Delete</button>
            </td>
        </tr>
    `).join("");
}

function renderBillingSelect() {
    elements.billingCustomer.innerHTML = state.customers.map((customer) => (
        `<option value="${customer.id}">${customer.id} - ${escapeHtml(customer.name)} (${escapeHtml(customer.type)})</option>`
    )).join("");

    if (state.previewCustomerId) {
        elements.billingCustomer.value = String(state.previewCustomerId);
    }
    syncBillingCustomer();
}

function resetSelectedCustomer() {
    elements.selectedCustomerName.textContent = "No customer selected";
    elements.selectedCustomerType.textContent = "-";
    elements.selectedCustomerStatus.textContent = "-";
    elements.selectedCustomerStatus.className = "";
    elements.selectedCustomerBill.textContent = formatCurrency(0);
    elements.previousReading.value = "";
    elements.currentReading.value = "";
    updatePreview(0);
}

function syncBillingCustomer() {
    const customer = state.customers.find((entry) => entry.id === Number(elements.billingCustomer.value));
    if (!customer) {
        resetSelectedCustomer();
        return;
    }

    state.previewCustomerId = customer.id;
    elements.previousReading.value = Number(customer.currentReading).toFixed(2);
    elements.currentReading.value = Number(customer.currentReading).toFixed(2);
    elements.selectedCustomerName.textContent = `${customer.id} - ${customer.name}`;
    elements.selectedCustomerType.textContent = customer.type;
    elements.selectedCustomerStatus.textContent = customer.isPaid ? "Paid" : "Pending";
    elements.selectedCustomerStatus.className = customer.isPaid ? "paid-text" : "pending-text";
    elements.selectedCustomerBill.textContent = formatCurrency(customer.billAmount);
    updatePreview(customer.currentReading);
}

function updatePreview(readingValue) {
    const customer = state.customers.find((entry) => entry.id === state.previewCustomerId);
    if (!customer) {
        elements.previewStatus.textContent = "No account";
        return;
    }

    const current = Number(readingValue);
    const previous = Number(customer.currentReading);
    if (Number.isNaN(current) || current < previous) {
        elements.previewStatus.textContent = "Check reading";
        elements.previewStatus.className = "status-pill status-pending";
        return;
    }

    const bill = calculatePreview(previous, current, customer.type);
    elements.previewUnits.textContent = bill.units.toFixed(2);
    elements.previewEnergy.textContent = formatCurrency(bill.energy);
    elements.previewFixed.textContent = formatCurrency(bill.fixed);
    elements.previewTax.textContent = formatCurrency(bill.tax);
    elements.previewTotal.textContent = formatCurrency(bill.total);
    elements.previewStatus.textContent = bill.units > 0 ? "Preview updated" : "Ready";
    elements.previewStatus.className = "status-pill status-paid";
}

function renderTariffs() {
    const types = ["Domestic", "Commercial", "Industrial"];
    elements.tariffCards.innerHTML = types.map((type) => `
        <article class="tariff-card">
            <span class="tariff-code">${type.slice(0, 3).toUpperCase()}</span>
            <p class="eyebrow">${type}</p>
            <h3>${formatCurrency(state.tariffs[type])}</h3>
            <p>Per unit, plus ${formatCurrency(state.tariffs.fixedCharge)} fixed charge and ${(state.tariffs.taxRate * 100).toFixed(0)}% tax.</p>
        </article>
    `).join("");

    $("tariffDomestic").value = Number(state.tariffs.Domestic).toFixed(2);
    $("tariffCommercial").value = Number(state.tariffs.Commercial).toFixed(2);
    $("tariffIndustrial").value = Number(state.tariffs.Industrial).toFixed(2);
}

function renderChart() {
    const totals = state.report.segmentTotals || {};
    const segments = ["Domestic", "Commercial", "Industrial"].map((type) => ({
        type,
        total: Number(totals[type] || 0)
    }));
    const maxValue = Math.max(...segments.map((segment) => segment.total), 1);
    const grandTotal = Math.max(segments.reduce((sum, segment) => sum + segment.total, 0), 1);

    elements.collectionChart.innerHTML = segments.map((segment) => {
        const barWidth = Math.max(2, (segment.total / maxValue) * 100);
        const percent = (segment.total / grandTotal) * 100;
        return `
            <div class="chart-bar">
                <div class="chart-label">
                    <span class="eyebrow">${segment.type}</span>
                    <strong>${formatCurrency(segment.total)}</strong>
                </div>
                <div class="chart-track">
                    <div class="chart-bar-fill" style="width: ${barWidth}%"></div>
                </div>
                <small class="eyebrow" style="margin-top: 4px; display: block; text-align: right; opacity: 0.5;">
                    ${percent.toFixed(1)}% OF TOTAL
                </small>
            </div>
        `;
    }).join("");
}

function renderAll() {
    renderCustomerTable();
    renderBillingSelect();
    renderMetrics();
    renderTariffs();
    renderChart();
}

function wireEvents() {
    elements.customerSearch.addEventListener("input", renderCustomerTable);
    elements.billingCustomer.addEventListener("change", syncBillingCustomer);
    elements.currentReading.addEventListener("input", (event) => updatePreview(event.target.value));

    elements.billingForm.addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            setBackendStatus("syncing", "Generating bill...");
            const data = await requestJson(api.generateBill, {
                method: "POST",
                body: JSON.stringify({
                    customerId: Number(elements.billingCustomer.value),
                    currentReading: Number(elements.currentReading.value)
                })
            });
            applyState(data.state);
            setBackendStatus("online", "Records synced");
            showToast(data.message || "Bill generated.", "success");
        } catch (error) {
            setBackendStatus("online", "Records synced");
            showToast(error.message, "error");
        }
    });

    elements.customerForm.addEventListener("submit", async (event) => {
        event.preventDefault();
        const payload = {
            name: $("customerName").value.trim(),
            address: $("customerAddress").value.trim(),
            contact: $("customerContact").value.trim(),
            type: $("customerType").value,
            previousReading: Number($("customerPrevious").value),
            currentReading: Number($("customerCurrent").value)
        };

        try {
            setBackendStatus("syncing", "Creating account...");
            const data = await requestJson(api.customers, {
                method: "POST",
                body: JSON.stringify(payload)
            });
            event.target.reset();
            applyState(data.state);
            setBackendStatus("online", "Records synced");
            showToast(data.message || "Customer created.", "success");
        } catch (error) {
            setBackendStatus("online", "Records synced");
            showToast(error.message, "error");
        }
    });

    elements.tariffForm.addEventListener("submit", async (event) => {
        event.preventDefault();
        try {
            setBackendStatus("syncing", "Saving tariffs...");
            const data = await requestJson(api.tariffs, {
                method: "POST",
                body: JSON.stringify({
                    Domestic: Number($("tariffDomestic").value),
                    Commercial: Number($("tariffCommercial").value),
                    Industrial: Number($("tariffIndustrial").value)
                })
            });
            applyState(data.state);
            setBackendStatus("online", "Records synced");
            showToast(data.message || "Tariffs updated.", "success");
        } catch (error) {
            setBackendStatus("online", "Records synced");
            showToast(error.message, "error");
        }
    });

    elements.customerTableBody.addEventListener("click", async (event) => {
        const button = event.target.closest("button[data-action]");
        if (!button) {
            return;
        }

        const customerId = Number(button.dataset.id);
        const customer = state.customers.find((entry) => entry.id === customerId);
        if (!customer) {
            return;
        }

        try {
            if (button.dataset.action === "toggle-paid") {
                setBackendStatus("syncing", "Updating payment...");
                const data = await requestJson(api.payments, {
                    method: "POST",
                    body: JSON.stringify({ customerId, isPaid: !customer.isPaid })
                });
                applyState(data.state);
                showToast(data.message || "Payment updated.", "success");
            }

            if (button.dataset.action === "delete") {
                setBackendStatus("syncing", "Deleting customer...");
                const data = await requestJson(`${api.customers}/${customerId}`, { method: "DELETE" });
                applyState(data.state);
                showToast(data.message || "Customer deleted.", "success");
            }

            setBackendStatus("online", "Records synced");
        } catch (error) {
            setBackendStatus("online", "Records synced");
            showToast(error.message, "error");
        }
    });

    $("openQuickBill").addEventListener("click", () => {
        $("billing").scrollIntoView({ behavior: "smooth", block: "start" });
        elements.billingCustomer.focus();
    });

    $("focusCustomers").addEventListener("click", () => {
        $("customers").scrollIntoView({ behavior: "smooth", block: "start" });
        elements.customerSearch.focus();
    });

    $("scrollToForm").addEventListener("click", () => {
        $("customerFormSection").scrollIntoView({ behavior: "smooth", block: "start" });
        $("customerName").focus();
    });

    document.querySelectorAll(".nav-link").forEach((button) => {
        button.addEventListener("click", () => {
            document.querySelectorAll(".nav-link").forEach((link) => link.classList.remove("is-active"));
            button.classList.add("is-active");
            $(button.dataset.target)?.scrollIntoView({ behavior: "smooth", block: "start" });
        });
    });
}

wireEvents();
loadState();
