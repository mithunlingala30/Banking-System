# 🏛️ MERIDIAN BANK &mdash; Sovereign Ledger & Banking System

> **A High-Performance, Auditable Digital Banking Platform & Plain-Text Ledger Engine**  
> *Built for Hackathons, Academic Demonstration, and Next-Generation Fintech Architecture.*

---

## 🌟 Executive Summary

**Meridian Bank** is a modern neo-fintech banking ecosystem powered by a dual-runtime backend (**High-Speed Native C Engine + Python REST API**) and a luxury dark-mode web application.

Designed with cryptographic integrity and mathematical precision, Meridian features:
- **Dual-Identifier Authentication**: Log in using either your unique **Account ID** (e.g. `ACC1003`) or your registered **Full Name** (e.g. `Puneeth Sai`).
- **Dedicated Modular Banking Architecture**: Separate, purpose-built pages for every banking action (Dashboard, Deposit, P2P Transfers, Loan Schemes, Statements, Admin Audit).
- **Official Indian Bank Loan Engine**: Real-time **Simple Interest** calculation ($SI = \frac{P \times R \times T}{100}$) with precise **Maturity / Return Date** scheduling.
- **Rupee Currency Standard (₹)**: Comprehensive Indian Rupee denomination with localized `en-IN` number formatting.
- **Admin Transaction Drilldown**: Executive oversight console allowing instant inspection of any user's complete statement by simply clicking their name.

---

## 🏗️ System Architecture

```
                               ┌────────────────────────────────────────┐
                               │       Modern Neo-Fintech Frontend      │
                               │   (Vanilla HTML5 / CSS3 / ES6 JS)      │
                               └──────────────────┬─────────────────────┘
                                                  │ HTTP JSON Requests
                                                  ▼
                               ┌────────────────────────────────────────┐
                               │         FastAPI Dev Server (app.py)    │
                               │      (Subprocess Execution Layer)      │
                               └──────────────────┬─────────────────────┘
                                                  │ Subprocess Call (CLI)
                                                  ▼
                         ┌────────────────────────────────────────────────────┐
                         │               Core Banking Engine                  │
                         │   Primary: C Native Binary (bank_engine.exe)       │
                         │   Fallback: Pure-Python Engine (bank_engine_py.py) │
                         └────────────────────────┬───────────────────────────┘
                                                  │ Atomic File I/O
                                                  ▼
                         ┌────────────────────────────────────────────────────┐
                         │            Plain-Text Ledger Database              │
                         │   accounts.txt  |  transactions.txt  |  loans.txt  │
                         └────────────────────────────────────────────────────┘
```

---

## ✨ Core Features & Innovations

### 1. 🔑 Dual-Identifier Authentication
- Log in seamlessly with **Account ID** (e.g. `ACC1003`, `ADMIN`) or **Full Name** (e.g. `Puneeth Sai`, `Alice`).
- Case-insensitive matching with instant cryptographic session storage.
- One-click demo credentials for rapid hackathon evaluations.

### 2. 🏛️ Bank-Scheme Simple Interest Loan Engine
- **Linear Simple Interest Formula**:
  $$\text{Simple Interest (SI)} = \frac{P \times R \times T}{100}$$
  $$\text{Total Amount Payable} = P + SI$$
  $$\text{Monthly Installment (EMI)} = \frac{P + SI}{\text{Tenure in Months}}$$
  where $P$ is Principal in ₹, $R$ is Annual Interest Rate (% p.a.), and $T$ is Tenure in Years ($\text{Months} / 12$).
- **Exact Maturity & Return Date Calculation**:
  Dynamically projects the exact day, month, and year of return based on the application date and tenure.
- **Official Pre-configured Loan Schemes**:
  1. **Personal Flexi Loan** (10.5% p.a., 6–36 Months)
  2. **Home & Property Scheme** (7.2% p.a., 12–240 Months)
  3. **Vehicle & Auto Loan** (8.9% p.a., 12–84 Months)
  4. **Business Expansion Loan** (11.0% p.a., 6–60 Months)
  5. **Higher Education Loan** (6.5% p.a., 12–60 Months)
  6. **Gold & Asset Backed Loan** (8.0% p.a., 3–36 Months)
  7. **Custom Scheme** (Tailor-made Rate & Terms)

### 3. 💳 Dedicated Action Modules (Separate Pages)
| Page | Route | Description |
| :--- | :--- | :--- |
| **Welcome / Login** | [`index.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/index.html) | Dual login, new account generation, quick-fill demo chips |
| **Overview Dashboard** | [`dashboard.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/dashboard.html) | Liquidity overview in ₹, quick action grid, recent activity |
| **Deposit Funds** | [`deposit.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/deposit.html) | Deposit via UPI / IMPS / NEFT / CDM, preset chips, digital voucher |
| **Transfer Money** | [`transfer.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/transfer.html) | Instant P2P transfer in ₹, zero fees, recipient lookup, transfer slip |
| **Loan Center** | [`loan.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/loan.html) | Interactive SI calculator, return date computation, ratio visualizer |
| **Account Statement** | [`transactions.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/transactions.html) | Full audit ledger, type filters (Credits/Debits/Loans), print statement |
| **Admin Console** | [`admin.html`](file:///c:/Users/Mithu/Downloads/meridian_bank_system/bank_system/frontend/admin.html) | Loan underwriting, master ledger audit, customer transaction drilldown |

### 4. 🛡️ Executive Admin Console with User Drilldown
- Review and underwrite pending loan applications with 1-click **Approve & Disburse** or **Reject**.
- **Customer Transaction Drilldown**: Click on any customer's name or account ID in the *All Accounts* tab to instantly open their complete individual ledger with inflow/outflow totals.

---

## 📁 Repository Directory Structure

```
bank_system/
├── backend/
│   ├── bank_engine.c          # Core C banking engine (compiled binary)
│   └── bank_engine_py.py       # Pure-Python drop-in replacement engine
├── data/
│   ├── accounts.txt           # id|name|password|balance|status|role
│   ├── transactions.txt       # id|timestamp|from|to|amount|type|note
│   ├── loans.txt              # id|account_id|amount|reason|status|timestamp
│   └── counters.txt           # next_acc_id|next_txn_id|next_loan_id
├── dev-server/
│   ├── app.py                 # FastAPI REST server & routing layer
│   └── requirements.txt       # Python dependencies (fastapi, uvicorn)
├── frontend/
│   ├── index.html             # Landing & Dual Auth portal
│   ├── dashboard.html         # User overview dashboard
│   ├── deposit.html           # Dedicated deposit page
│   ├── transfer.html          # Dedicated transfer page
│   ├── loan.html              # Loan schemes & SI calculator
│   ├── transactions.html      # Statement & transaction history
│   ├── admin.html             # Admin console & user statement audit
│   ├── style.css              # Neo-Fintech design system & tokens
│   └── app.js                 # Shared frontend logic & loan math engine
└── README.md                  # Project documentation & presentation guide
```

---

## 🚀 Quick Start Guide

### Prerequisites
- **Python 3.8+** installed
- Optional: `gcc` compiler (if C engine compilation is desired; otherwise automatically runs with the Python engine)

### 1. Installation
```powershell
cd c:\Users\Mithu\Downloads\meridian_bank_system\bank_system\dev-server
pip install -r requirements.txt
```

### 2. Run Dev Server
```powershell
uvicorn app:app --reload --port 8000
```

### 3. Open in Browser
Visit **[http://127.0.0.1:8000/index.html](http://127.0.0.1:8000/index.html)**

---

## ⚡ Demo & Testing Credentials

| Role | Account ID | Username | Password | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| **Customer** | `ACC1003` | `Puneeth Sai` | `123456` | Primary demo customer (₹1,00,000 balance) |
| **Customer** | `ACC1001` | `Alice` | `pw1` | Secondary recipient for transfers |
| **Admin** | `ADMIN` | `ADMIN` | `admin123` | Executive console for loan approvals & audit |

---

## 🎤 2-Minute Hackathon Presentation Walkthrough

1. **Sign-In & Onboarding (`index.html`)**:
   - Demonstrate **Dual Login** by signing in using either `Puneeth Sai` (Username) or `ACC1003` (Account ID).
2. **Dashboard & Liquidity (`dashboard.html`)**:
   - Showcase the **₹ Balance Hero Card** and the dedicated **Banking Action Hub**.
3. **Loan Scheme & Return Date Simulator (`loan.html`)**:
   - Select a bank scheme (e.g. *Vehicle & Auto Loan* at 8.9% p.a.).
   - Adjust the Principal (₹) and Tenure sliders.
   - Highlight the **Exact Return Date** calculation and visual ratio bar.
   - Submit the loan application.
4. **Admin Approval & Disbursal (`admin.html`)**:
   - Log into the Admin Console with `ADMIN` / `admin123`.
   - Underwrite and **Approve** the pending loan; watch the principal disburse into the ledger.
   - In *All Accounts*, click on `Puneeth Sai` to reveal the **Individual Transaction Statement Modal**.
5. **Instant Peer-to-Peer Transfer (`transfer.html`)**:
   - Send ₹5,000 to `Alice` (`ACC1001`) with zero transaction fees and instant ledger slip generation.

---

## 🛡️ License & Academic Integrity
Developed for educational demonstration, hackathon innovation, and fintech architecture prototyping.
All ledger data is stored locally in plain-text pipe-delimited records for transparent auditability.
#   B a n k i n g - S y s t e m  
 #   B a n k i n g - S y s t e m  
 