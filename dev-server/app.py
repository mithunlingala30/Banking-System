"""
app.py -- Meridian Bank API Server

Engine selection:
  1. Try C engine compilation if gcc works.
  2. Fall back to pure-Python engine (identical interface and data format).
"""

import json
import subprocess
import sys
from pathlib import Path
from typing import Optional, Union, Any

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from fastapi.responses import JSONResponse
from pydantic import BaseModel

BASE_DIR     = Path(__file__).resolve().parent.parent
BACKEND_DIR  = BASE_DIR / "backend"
DATA_DIR     = BASE_DIR / "data"
FRONTEND_DIR = BASE_DIR / "frontend"
ENGINE_SRC   = BACKEND_DIR / "bank_engine.c"
ENGINE_BIN   = BACKEND_DIR / "bank_engine.exe"
ENGINE_PY    = BACKEND_DIR / "bank_engine_py.py"

_engine_mode  = "python"
_engine_label = "Python Engine"
_engine_note  = ""

def _try_compile_c() -> bool:
    need_build = (not ENGINE_BIN.exists()) or (
        ENGINE_SRC.stat().st_mtime > ENGINE_BIN.stat().st_mtime
    )
    if not need_build and ENGINE_BIN.exists():
        return True

    print("  Compiling C backend (bank_engine.c) ...")
    try:
        result = subprocess.run(
            ["gcc", "-O2", "-Wall", "-o", str(ENGINE_BIN), str(ENGINE_SRC)],
            cwd=str(BACKEND_DIR), capture_output=True, text=True, timeout=15,
        )
        if result.returncode == 0 and ENGINE_BIN.exists():
            return True
        return False
    except Exception:
        return False

def select_engine() -> None:
    global _engine_mode, _engine_label, _engine_note
    DATA_DIR.mkdir(exist_ok=True)
    if _try_compile_c():
        _engine_mode  = "c"
        _engine_label = "C Native Engine"
        _engine_note  = f"Binary: {ENGINE_BIN}"
        print(f"  [ENGINE] Running with C backend -> {ENGINE_BIN}")
    else:
        _engine_mode  = "python"
        _engine_label = "Python Backend Engine"
        _engine_note  = f"Script: {ENGINE_PY}"
        print(f"  [ENGINE] Running with Python backend -> {ENGINE_PY}")

def call_engine(*args) -> dict:
    if _engine_mode == "c" and ENGINE_BIN.exists():
        cmd = [str(ENGINE_BIN), *[str(a) for a in args]]
    else:
        cmd = [sys.executable, str(ENGINE_PY), *[str(a) for a in args]]

    try:
        result = subprocess.run(
            cmd, cwd=str(BASE_DIR), capture_output=True, text=True, timeout=10,
        )
        out = result.stdout.strip()
        if not out:
            stderr = result.stderr.strip()
            return {"status": "error", "message": f"engine_no_output: {stderr}"}
        return json.loads(out)
    except json.JSONDecodeError:
        return {"status": "error", "message": "engine_bad_json"}
    except Exception as exc:
        return {"status": "error", "message": str(exc)}

def engine_response(result: dict) -> JSONResponse:
    status_code = 200 if result.get("status") == "ok" else 400
    return JSONResponse(content=result, status_code=status_code)

app = FastAPI(title="Meridian Bank API")

@app.on_event("startup")
def on_startup():
    select_engine()
    print("  Admin credentials: ID/Name: ADMIN | Password: admin123")

@app.get("/api/engine")
def engine_info():
    return JSONResponse(content={
        "status": "ok",
        "mode": _engine_mode,
        "label": _engine_label,
        "note": _engine_note,
        "c_bin_exists": ENGINE_BIN.exists(),
    })

# ---------------------------------------------------------------------------
# Request Models
# ---------------------------------------------------------------------------

class SignupBody(BaseModel):
    name: str
    password: str
    initial_deposit: Optional[Union[str, float, int]] = "0"

class LoginBody(BaseModel):
    account_id: Optional[str] = None
    username: Optional[str] = None
    identifier: Optional[str] = None
    password: str

class DepositBody(BaseModel):
    account_id: str
    password: str
    amount: Union[str, float, int]

class TransferBody(BaseModel):
    account_id: str
    password: str
    to_id: str
    amount: Union[str, float, int]

class LoanRequestBody(BaseModel):
    account_id: str
    password: str
    amount: Union[str, float, int]
    scheme: Optional[str] = "Personal Flexi Loan"
    interest_rate: Optional[Union[str, float, int]] = "8.5"
    tenure_months: Optional[Union[str, int]] = "12"
    total_payable: Optional[Union[str, float, int]] = None
    return_date: Optional[str] = None
    reason: Optional[str] = None

class LoanDecisionBody(BaseModel):
    password: str
    loan_id: Union[str, int]
    decision: str  # "approve" | "reject"

# ---------------------------------------------------------------------------
# Routes
# ---------------------------------------------------------------------------

@app.post("/api/signup")
def signup(body: SignupBody):
    deposit = str(body.initial_deposit or "0")
    return engine_response(call_engine("signup", body.name, body.password, deposit))

@app.post("/api/login")
def login(body: LoginBody):
    ident = (body.identifier or body.account_id or body.username or "").strip()
    return engine_response(call_engine("login", ident, body.password))

@app.post("/api/deposit")
def deposit(body: DepositBody):
    return engine_response(call_engine("deposit", body.account_id, body.password, str(body.amount)))

@app.post("/api/transfer")
def transfer(body: TransferBody):
    return engine_response(call_engine("transfer", body.account_id, body.password, body.to_id, str(body.amount)))

@app.post("/api/loan/request")
def loan_request(body: LoanRequestBody):
    scheme = body.scheme or "Personal Flexi Loan"
    rate = str(body.interest_rate or "8.5")
    tenure = str(body.tenure_months or "12")
    user_reason = body.reason or "General Banking Purpose"
    
    # Calculate or format simple interest return details
    try:
        p = float(body.amount)
        r = float(rate)
        t = float(tenure)
        si = (p * r * (t / 12.0)) / 100.0
        tot = p + si
        emi = tot / t if t > 0 else tot
        total_str = f"{tot:.2f}"
        emi_str = f"{emi:.2f}"
    except Exception:
        total_str = str(body.total_payable or body.amount)
        emi_str = "-"

    ret_date = body.return_date or "Standard Term"
    formatted_reason = f"[{scheme}] {rate}% p.a. | {tenure} Mo (EMI: ₹{emi_str}, Total: ₹{total_str}, Due: {ret_date}) - {user_reason}"
    
    return engine_response(call_engine("loan_request", body.account_id, body.password, str(body.amount), formatted_reason))

@app.get("/api/account")
def get_account(account_id: str, password: str):
    return engine_response(call_engine("get_account", account_id, password))

@app.get("/api/loans/mine")
def get_my_loans(account_id: str, password: str):
    return engine_response(call_engine("get_loans_for", account_id, password))

@app.get("/api/admin/accounts")
def admin_accounts(password: str):
    return engine_response(call_engine("admin_accounts", password))

@app.get("/api/admin/transactions")
def admin_transactions(password: str):
    return engine_response(call_engine("admin_transactions", password))

@app.get("/api/admin/loans/pending")
def admin_loans_pending(password: str):
    return engine_response(call_engine("admin_loans_pending", password))

@app.post("/api/admin/loans/decide")
def admin_loans_decide(body: LoanDecisionBody):
    return engine_response(call_engine("admin_loan_decision", body.password, str(body.loan_id), body.decision))

app.mount("/", StaticFiles(directory=str(FRONTEND_DIR), html=True), name="frontend")
