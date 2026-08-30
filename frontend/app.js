/* ==========================================================================
   app.js — Meridian Bank Shared Utilities & Component Engine
   - Seamless API integration
   - Universal Navigation Bar with active tab indicators & user profile
   - Interactive Simple Interest Engine (SI = P * R * T / 100) & Return Date Calculator
   - Modern Toast Alert System & Receipt Modal Renderer
   - Formats currency in Indian Rupees (₹)
   ========================================================================== */

const API = {
    async post(path, body) {
        try {
            const res = await fetch(path, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(body || {}),
            });
            return await res.json();
        } catch (err) {
            console.error("API POST error:", err);
            return { status: "error", message: "Network or server connection failure" };
        }
    },
    async get(path, params) {
        try {
            const qs = new URLSearchParams(params || {}).toString();
            const res = await fetch(path + (qs ? "?" + qs : ""));
            return await res.json();
        } catch (err) {
            console.error("API GET error:", err);
            return { status: "error", message: "Network or server connection failure" };
        }
    },
};

const Session = {
    save(account_id, password, role, name) {
        localStorage.setItem("bank_session", JSON.stringify({ account_id, password, role, name }));
    },
    get() {
        const raw = localStorage.getItem("bank_session");
        return raw ? JSON.parse(raw) : null;
    },
    clear() {
        localStorage.removeItem("bank_session");
    },
    requireUser() {
        const s = Session.get();
        if (!s) {
            window.location.href = "index.html";
            return null;
        }
        return s;
    },
    requireAdmin() {
        const s = Session.get();
        if (!s || s.role !== "admin") {
            window.location.href = "index.html";
            return null;
        }
        return s;
    },
};

/* ---------- Toast Notification System ---------- */
const Toast = {
    container: null,
    init() {
        if (!this.container) {
            this.container = document.createElement("div");
            this.container.className = "toast-container";
            document.body.appendChild(this.container);
        }
    },
    show(message, type = "info", duration = 4000) {
        this.init();
        const toast = document.createElement("div");
        toast.className = `toast toast-${type}`;
        
        let icon = "ℹ️";
        if (type === "success") icon = "✅";
        if (type === "error") icon = "❌";
        
        toast.innerHTML = `<span>${icon}</span> <span>${escapeHtml(message)}</span>`;
        this.container.appendChild(toast);
        
        setTimeout(() => {
            toast.style.opacity = "0";
            toast.style.transform = "translateY(10px)";
            toast.style.transition = "all 0.3s ease";
            setTimeout(() => toast.remove(), 300);
        }, duration);
    },
    success(msg) { this.show(msg, "success"); },
    error(msg) { this.show(msg, "error"); },
    info(msg) { this.show(msg, "info"); }
};

/* ---------- Formatting Helpers (Rupees ₹ & Indian Number System) ---------- */
function fmtMoney(n) {
    const num = Number(n || 0);
    return num.toLocaleString("en-IN", { minimumFractionDigits: 2, maximumFractionDigits: 2 });
}

function escapeHtml(str) {
    const div = document.createElement("div");
    div.textContent = str == null ? "" : String(str);
    return div.innerHTML;
}

function copyToClipboard(text, label = "Copied to clipboard!") {
    navigator.clipboard.writeText(text).then(() => {
        Toast.success(label);
    }).catch(() => {
        Toast.info("Text: " + text);
    });
}

/* ---------- Bank Scheme & Simple Interest Engine ---------- */
const LoanSchemes = [
    {
        id: "personal",
        name: "Personal Flexi Loan",
        rate: 10.5,
        defaultTenure: 12,
        defaultAmount: 50000,
        minTenure: 6,
        maxTenure: 36,
        description: "Instant disbursement for personal needs with low fixed interest."
    },
    {
        id: "home",
        name: "Home & Property Scheme",
        rate: 7.2,
        defaultTenure: 60,
        defaultAmount: 2500000,
        minTenure: 12,
        maxTenure: 240,
        description: "Lowest subsidized rate for real estate and home investments."
    },
    {
        id: "auto",
        name: "Vehicle & Auto Loan",
        rate: 8.9,
        defaultTenure: 24,
        defaultAmount: 500000,
        minTenure: 12,
        maxTenure: 84,
        description: "Drive your dream vehicle with flexible tenure & fixed return schedule."
    },
    {
        id: "business",
        name: "Business Expansion Loan",
        rate: 11.0,
        defaultTenure: 24,
        defaultAmount: 1000000,
        minTenure: 6,
        maxTenure: 60,
        description: "Working capital and equipment financing for growing ventures."
    },
    {
        id: "education",
        name: "Higher Education Loan",
        rate: 6.5,
        defaultTenure: 36,
        defaultAmount: 400000,
        minTenure: 12,
        maxTenure: 60,
        description: "Special academic scheme with student-friendly grace terms."
    },
    {
        id: "gold",
        name: "Gold & Asset Backed Loan",
        rate: 8.0,
        defaultTenure: 12,
        defaultAmount: 200000,
        minTenure: 3,
        maxTenure: 36,
        description: "Instant liquidity backed by physical gold with minimal paperwork."
    },
    {
        id: "custom",
        name: "Custom Banking Scheme",
        rate: 9.0,
        defaultTenure: 12,
        defaultAmount: 100000,
        minTenure: 1,
        maxTenure: 120,
        description: "Tailor your own interest rate and repayment timeline."
    }
];

/**
 * Calculates Simple Interest, Total Repayment, Monthly Installment, and Exact Return Date
 * Formula: SI = (P * R * T) / 100
 * where T is in years (tenureMonths / 12)
 */
function calculateSimpleInterestLoan(principal, annualRatePercent, tenureMonths) {
    const P = Math.max(0, parseFloat(principal) || 0);
    const R = Math.max(0, parseFloat(annualRatePercent) || 0);
    const M = Math.max(1, parseInt(tenureMonths) || 1);
    
    const T_years = M / 12.0;
    const simpleInterest = (P * R * T_years) / 100.0;
    const totalRepayment = P + simpleInterest;
    const monthlyEMI = totalRepayment / M;
    
    // Calculate exact return / maturity date
    const returnDateObj = new Date();
    returnDateObj.setMonth(returnDateObj.getMonth() + M);
    
    const options = { year: 'numeric', month: 'short', day: 'numeric' };
    const exactReturnDate = returnDateObj.toLocaleDateString('en-IN', options);
    
    const principalPercent = totalRepayment > 0 ? (P / totalRepayment) * 100 : 100;
    const interestPercent = totalRepayment > 0 ? (simpleInterest / totalRepayment) * 100 : 0;
    
    return {
        principal: P,
        annualRate: R,
        tenureMonths: M,
        tenureYears: T_years,
        simpleInterest: simpleInterest,
        totalRepayment: totalRepayment,
        monthlyEMI: monthlyEMI,
        exactReturnDate: exactReturnDate,
        returnDateISO: returnDateObj.toISOString().split('T')[0],
        principalPercent: principalPercent,
        interestPercent: interestPercent
    };
}

/* ---------- Universal Navigation Bar Component ---------- */
function renderNavbar(activeTab = "dashboard") {
    const session = Session.get();
    const isAuth = !!session;
    const isAdmin = session && session.role === "admin";
    
    const navEl = document.createElement("nav");
    navEl.className = "navbar";
    
    const homeUrl = !isAuth ? "index.html" : (isAdmin ? "admin.html" : "dashboard.html");
    
    let linksHtml = "";
    if (isAuth && !isAdmin) {
        linksHtml = `
            <ul class="nav-links">
                <li><a href="dashboard.html" class="nav-link ${activeTab === 'dashboard' ? 'active' : ''}"><span>🏠</span> Dashboard</a></li>
                <li><a href="deposit.html" class="nav-link ${activeTab === 'deposit' ? 'active' : ''}"><span>💳</span> Deposit</a></li>
                <li><a href="transfer.html" class="nav-link ${activeTab === 'transfer' ? 'active' : ''}"><span>💸</span> Transfer</a></li>
                <li><a href="loan.html" class="nav-link ${activeTab === 'loan' ? 'active' : ''}"><span>🏛️</span> Loan Center</a></li>
                <li><a href="transactions.html" class="nav-link ${activeTab === 'transactions' ? 'active' : ''}"><span>📜</span> Statement</a></li>
            </ul>
        `;
    } else if (isAuth && isAdmin) {
        linksHtml = `
            <ul class="nav-links">
                <li><a href="admin.html" class="nav-link ${activeTab === 'admin' ? 'active' : ''}"><span>🛡️</span> Admin Console</a></li>
                <li><a href="dashboard.html" class="nav-link"><span>👁️</span> User View</a></li>
            </ul>
        `;
    }
    
    let userHtml = "";
    if (isAuth) {
        const initials = (session.name || "U").substring(0, 2).toUpperCase();
        userHtml = `
            <div class="nav-user">
                <div class="user-chip">
                    <div class="user-avatar">${initials}</div>
                    <div class="user-details">
                        <span class="user-name">${escapeHtml(session.name)}</span>
                        <span class="user-acc-id" onclick="copyToClipboard('${session.account_id}', 'Account ID ${session.account_id} copied!')" title="Click to copy Account ID">${session.account_id} 📋</span>
                    </div>
                </div>
                <button class="btn btn-ghost btn-sm" onclick="logout()" title="Sign out of Meridian">⏻ Log out</button>
            </div>
        `;
    } else {
        userHtml = `
            <div class="nav-user">
                <a href="index.html" class="btn btn-gold btn-sm">Access Account</a>
            </div>
        `;
    }
    
    navEl.innerHTML = `
        <div class="nav-container">
            <a href="${homeUrl}" class="brand">
                <div class="brand-icon">M</div>
                <span>MERIDIAN</span>
                ${isAdmin ? '<span class="brand-badge">ADMIN</span>' : ''}
            </a>
            ${linksHtml}
            ${userHtml}
        </div>
    `;
    
    document.body.prepend(navEl);
}

function logout() {
    Session.clear();
    Toast.info("Signed out successfully");
    setTimeout(() => {
        window.location.href = "index.html";
    }, 400);
}
