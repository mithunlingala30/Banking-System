# 🏛️ Meridian Banking System

<p align="center">
  <img src="https://img.shields.io/badge/Architecture-C%20Native%20%2B%20Python%20FastAPI-d4af37?style=for-the-badge" alt="Architecture" />
  <img src="https://img.shields.io/badge/Currency-INR%20(%E2%82%B9)-10b981?style=for-the-badge" alt="Currency" />
  <img src="https://img.shields.io/badge/Ledger-Plain--Text%20Audit%20Trail-6366f1?style=for-the-badge" alt="Ledger" />
  <img src="https://img.shields.io/badge/Frontend-Vanilla%20HTML5%2FCSS3%2FJS-f59e0b?style=for-the-badge" alt="Frontend" />
</p>

---

## 🌟 Overview

**Meridian Banking System** is a full-featured digital banking platform built for high-throughput transactional auditing, loan underwriting, and peer-to-peer liquidity settlement.

Powered by a **C Native / Python Dual-Engine Architecture** and a responsive **Neo-Fintech UI**, Meridian replaces bulky databases with an atomic, transparent, plain-text ledger system.

---

## 🚀 Key Highlights & Features

### 🔑 1. Dual-Identifier Authentication
Log in seamlessly using either:
- **Account ID** (e.g., `ACC1003`, `ACC1001`, `ADMIN`)
- **Full Legal Name** (e.g., `Puneeth Sai`, `Alice`, `Bank Administrator`)

### 🏛️ 2. Bank-Scheme Simple Interest Loan Engine
Calculate loan repayments, interest rates, and exact maturity return dates with banking precision:
- **Linear Simple Interest Formula**:
  $$\text{Simple Interest (SI)} = \frac{P \times R \times T}{100}$$
  $$\text{Total Repayment Amount} = P + SI$$
  $$\text{Monthly EMI} = \frac{P + SI}{\text{Tenure (Months)}}$$
  *(where $P$ = Principal in ₹, $R$ = Annual Rate %, and $T = \text{Tenure in Months} / 12$)*

- **Exact Maturity / Return Date**: Dynamically computed down to the exact day (e.g., 12-month tenure applied on Aug 30, 2026 $\rightarrow$ Return Due: **Aug 30, 2027**).

- **6 Pre-Configured Indian Banking Schemes**:
  1. **Personal Flexi Loan** (10.5% p.a., 6–36 Months)
  2. **Home & Property Loan** (7.2% p.a., 12–240 Months)
  3. **Vehicle & Auto Loan** (8.9% p.a., 12–84 Months)
  4. **Business Expansion Loan** (11.0% p.a., 6–60 Months)
  5. **Higher Education Loan** (6.5% p.a., 12–60 Months)
  6. **Gold & Asset Backed Loan** (8.0% p.a., 3–36 Months)
  7. **Custom Scheme** (Flexible Principal, Rate & Tenure)

### 💳 3. Dedicated Action Modules (Separate Pages)
Each banking action is decoupled into a dedicated, focused interface:

| Module | File Link | Key Capabilities |
| :--- | :--- | :--- |
| **Auth & Onboarding** | [`frontend/index.html`](frontend/index.html) | Dual login (Username/ID), instant account generator, quick-fill demo chips |
| **Account Dashboard** | [`frontend/dashboard.html`](frontend/dashboard.html) | Real-time liquidity card (₹), action navigation hub, recent ledger preview |
| **Deposit Portal** | [`frontend/deposit.html`](frontend/deposit.html) | UPI/IMPS/CDM deposit channels, quick-preset chips (₹500 to ₹50,000), instant receipt |
| **P2P Transfer** | [`frontend/transfer.html`](frontend/transfer.html) | Zero-fee remittance, recipient lookup (by Name or ID), transfer slip modal |
| **Loan Center** | [`frontend/loan.html`](frontend/loan.html) | Interactive SI calculator, return date computation, ratio visualizer, application tracker |
| **Account Statement** | [`frontend/transactions.html`](frontend/transactions.html) | Filterable transaction audit trail (Inflows, Outflows, Loans), search & print |
| **Admin Console** | [`frontend/admin.html`](frontend/admin.html) | 1-click loan underwriting, customer transaction drilldowns by clicking user names |

### 🛡️ 4. Executive Admin Console with User Ledger Drilldown
- **Customer Audit Drilldown**: Click on any customer's name or account ID to instantly open their complete transaction statement modal with inflow/outflow breakdown.
- **Underwriting Suite**: Inspect loan scheme terms, principal, simple interest, and exact return dates before 1-click **Approve & Disburse** or **Reject**.

---

## 🏗️ System Architecture

```
┌────────────────────────────────────────────────────────────────────────┐
│                      Client Layer (HTML5 / CSS3 / ES6)                 │
│  index.html  │  dashboard.html  │  deposit.html  │  transfer.html      │
│  loan.html   │  transactions.html  │  admin.html  │  style.css         │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ JSON API Calls (fetch)
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                  Application Gateway (FastAPI / Uvicorn)               │
│               Endpoint routing, input validation & dispatch            │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ Subprocess Execution Layer
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                   Dual Core Banking Engine Runtime                     │
│   • Primary: Compiled C99 Native Binary (backend/bank_engine.c)        │
│   • Fallback: Pure-Python Engine (backend/bank_engine_py.py)           │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ Synchronous Atomic File I/O
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                     Plain-Text Ledger Storage (data/)                  │
│   accounts.txt   │   transactions.txt   │   loans.txt   │ counters.txt │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 📁 Repository Structure

```
bank_system/
├── backend/
│   ├── bank_engine.c          # High-performance C core banking binary
│   └── bank_engine_py.py       # Pure-Python drop-in banking engine
├── data/
│   ├── accounts.txt           # id|name|password|balance|status|role
│   ├── transactions.txt       # id|timestamp|from|to|amount|type|note
│   ├── loans.txt              # id|account_id|amount|reason|status|timestamp
│   └── counters.txt           # Auto-incrementing ledger sequence counters
├── dev-server/
│   ├── app.py                 # FastAPI server & static file host
│   └── requirements.txt       # Python dependencies (fastapi, uvicorn)
├── frontend/
│   ├── index.html             # Welcome, Dual Login & Account Opening
│   ├── dashboard.html         # Liquidity & financial overview
│   ├── deposit.html           # Dedicated deposit portal
│   ├── transfer.html          # Peer-to-peer transfer portal
│   ├── loan.html              # Loan scheme calculator & return scheduler
│   ├── transactions.html      # Account statement & audit ledger
│   ├── admin.html             # Executive console & user statement drilldown
│   ├── style.css              # Neo-Fintech design system & tokens
│   └── app.js                 # Shared frontend controller & loan math engine
└── README.md                  # Project documentation & presentation guide
```

---

## ⚡ Quick Start & Setup

### 1. Clone & Navigate
```bash
git clone https://github.com/mithunlingala30/Banking-System.git
cd Banking-System/dev-server
```

### 2. Install Dependencies
```bash
pip install -r requirements.txt
```

### 3. Launch Server
```bash
uvicorn app:app --reload --port 8000
```

### 4. Access Application
Open your browser and navigate to:
**[http://127.0.0.1:8000/index.html](http://127.0.0.1:8000/index.html)**

---

## 🧪 Demo Credentials

Use these pre-seeded accounts for testing and live demonstrations:

| Role | Account ID | Registered Name | Password | Initial Balance |
| :--- | :--- | :--- | :--- | :--- |
| **Primary User** | `ACC1003` | `Puneeth Sai` | `123456` | ₹1,00,000.00 |
| **Secondary User** | `ACC1001` | `Alice` | `pw1` | ₹2,200.00 |
| **Administrator** | `ADMIN` | `Bank Administrator` | `admin123` | Master Auditor |

---

## 🎤 2-Minute Presentation Walkthrough

1. **Dual-Identifier Sign In (`index.html`)**:
   - Demonstrate logging in with Username (`Puneeth Sai`) or Account ID (`ACC1003`).
2. **Dashboard Overview (`dashboard.html`)**:
   - Showcase the Rupee (₹) Liquidity Hero Card and quick action navigation grid.
3. **Interactive Simple Interest Loan Simulator (`loan.html`)**:
   - Select a scheme (*Vehicle & Auto Loan* at 8.9% p.a.).
   - Adjust the Principal (₹) and Tenure sliders to demonstrate dynamic **Simple Interest**, **Monthly EMI**, and **Exact Return Date** computation.
   - Submit the loan application.
4. **Executive Underwriting (`admin.html`)**:
   - Log in as `ADMIN`.
   - Inspect the pending loan's interest and return date breakdown, then click **Approve & Disburse**.
   - In *All Accounts*, click on `Puneeth Sai` to open the **Customer Statement Drilldown Modal**.
5. **Instant Peer Transfer (`transfer.html`)**:
   - Send ₹5,000 to `Alice` (`ACC1001`) with zero transaction fees and instant digital voucher generation.

---

## 🛡️ License & Academic Integrity
Developed for educational demonstration, hackathon presentation, and fintech architecture prototyping.
All ledger data is stored in plain-text pipe-delimited records for complete transparency and auditability.