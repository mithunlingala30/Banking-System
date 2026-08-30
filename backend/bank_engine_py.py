#!/usr/bin/env python3
"""
bank_engine_py.py
-----------------
Pure-Python drop-in replacement for bank_engine.c.

IDENTICAL interface:
  python bank_engine_py.py <action> [args...]

IDENTICAL pipe-delimited data files:
  data/accounts.txt     id|name|password|balance|status|role
  data/transactions.txt id|timestamp|from|to|amount|type|note
  data/loans.txt        id|account_id|amount|reason|status|timestamp
  data/counters.txt     next_acc_id|next_txn_id|next_loan_id

IDENTICAL JSON output - one line on stdout.
"""

import sys
import json
import time
from pathlib import Path
from typing import Optional

DATA_DIR        = Path("data")
ACCOUNTS_FILE   = DATA_DIR / "accounts.txt"
TXNS_FILE       = DATA_DIR / "transactions.txt"
LOANS_FILE      = DATA_DIR / "loans.txt"
COUNTERS_FILE   = DATA_DIR / "counters.txt"

def ok(**kwargs):
    print(json.dumps({"status": "ok", **kwargs}))

def err(message):
    print(json.dumps({"status": "error", "message": message}))

def ensure_data_files():
    DATA_DIR.mkdir(exist_ok=True)
    for f in [ACCOUNTS_FILE, TXNS_FILE, LOANS_FILE]:
        f.touch(exist_ok=True)
    if not COUNTERS_FILE.exists():
        COUNTERS_FILE.write_text("1001|1|1\n")

def read_counters():
    try:
        parts = COUNTERS_FILE.read_text().strip().split("|")
        return int(parts[0]), int(parts[1]), int(parts[2])
    except Exception:
        return 1001, 1, 1

def write_counters(acc, txn, loan):
    COUNTERS_FILE.write_text(f"{acc}|{txn}|{loan}\n")

def timestamp_str():
    return time.strftime("%Y-%m-%d %H:%M:%S")

def parse_account_line(line):
    parts = line.rstrip("\n").split("|")
    if len(parts) < 6:
        return None
    try:
        return {"id": parts[0], "name": parts[1], "password": parts[2],
                "balance": float(parts[3]), "status": parts[4], "role": parts[5]}
    except ValueError:
        return None

def find_account(account_id_or_name):
    query = str(account_id_or_name).strip().lower()
    try:
        for line in ACCOUNTS_FILE.read_text().splitlines():
            a = parse_account_line(line)
            if a and (a["id"].lower() == query or a["name"].lower() == query):
                return a
    except FileNotFoundError:
        pass
    return None

def append_account(id_, name, password, balance, status, role):
    with ACCOUNTS_FILE.open("a", encoding="utf-8") as f:
        f.write(f"{id_}|{name}|{password}|{balance:.2f}|{status}|{role}\n")

def update_account_balance(account_id, new_balance):
    try:
        lines = ACCOUNTS_FILE.read_text(encoding="utf-8").splitlines()
    except FileNotFoundError:
        return False
    new_lines = []
    found = False
    for line in lines:
        a = parse_account_line(line)
        if a and a["id"].lower() == str(account_id).lower():
            new_lines.append(f"{a['id']}|{a['name']}|{a['password']}|{new_balance:.2f}|{a['status']}|{a['role']}")
            found = True
        else:
            new_lines.append(line)
    ACCOUNTS_FILE.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
    return found

def append_transaction(txn_id, from_id, to_id, amount, type_, note):
    ts = timestamp_str()
    with TXNS_FILE.open("a", encoding="utf-8") as f:
        f.write(f"{txn_id}|{ts}|{from_id}|{to_id}|{amount:.2f}|{type_}|{note}\n")

def parse_txn_line(line):
    parts = line.rstrip("\n").split("|")
    if len(parts) < 7:
        return None
    try:
        return {"id": int(parts[0]), "time": parts[1], "from": parts[2],
                "to": parts[3], "amount": float(parts[4]), "type": parts[5], "note": parts[6]}
    except ValueError:
        return None

def parse_loan_line(line):
    parts = line.rstrip("\n").split("|")
    if len(parts) < 6:
        return None
    try:
        return {"loan_id": int(parts[0]), "account_id": parts[1], "amount": float(parts[2]),
                "reason": parts[3], "status": parts[4], "time": parts[5]}
    except ValueError:
        return None

def ensure_admin():
    if not find_account("ADMIN"):
        append_account("ADMIN", "Bank Administrator", "admin123", 0.0, "active", "admin")

def admin_check(password):
    a = find_account("ADMIN")
    return a is not None and a["password"] == password

def action_signup(name, password, initial_deposit):
    ensure_data_files(); ensure_admin()
    acc, txn, loan = read_counters()
    account_id = f"ACC{acc}"
    dep = max(0.0, float(initial_deposit))
    append_account(account_id, name, password, dep, "active", "user")
    if dep > 0:
        append_transaction(txn, "EXTERNAL", account_id, dep, "deposit", "initial_deposit")
        txn += 1
    write_counters(acc + 1, txn, loan)
    ok(account_id=account_id, name=name, balance=dep)

def action_login(account_id_or_name, password):
    ensure_data_files(); ensure_admin()
    a = find_account(account_id_or_name)
    if not a or a["password"] != password:
        err("invalid_credentials"); return
    if a["status"] != "active":
        err("account_not_active"); return
    ok(account_id=a["id"], name=a["name"], balance=a["balance"], role=a["role"])

def action_deposit(account_id_or_name, password, amount_s):
    ensure_data_files()
    a = find_account(account_id_or_name)
    if not a or a["password"] != password:
        err("invalid_credentials"); return
    amount = float(amount_s)
    if amount <= 0:
        err("invalid_amount"); return
    new_bal = a["balance"] + amount
    update_account_balance(a["id"], new_bal)
    acc, txn, loan = read_counters()
    append_transaction(txn, "EXTERNAL", a["id"], amount, "deposit", "cash_deposit")
    write_counters(acc, txn + 1, loan)
    ok(balance=new_bal)

def action_transfer(from_id_or_name, password, to_id_or_name, amount_s):
    ensure_data_files()
    from_acc = find_account(from_id_or_name)
    if not from_acc or from_acc["password"] != password:
        err("invalid_credentials"); return
    to_acc = find_account(to_id_or_name)
    if not to_acc:
        err("recipient_not_found"); return
    amount = float(amount_s)
    if amount <= 0:
        err("invalid_amount"); return
    if from_acc["balance"] < amount:
        err("insufficient_funds"); return
    if from_acc["id"] == to_acc["id"]:
        err("cannot_transfer_to_self"); return
    update_account_balance(from_acc["id"], from_acc["balance"] - amount)
    update_account_balance(to_acc["id"], to_acc["balance"] + amount)
    acc, txn, loan = read_counters()
    append_transaction(txn, from_acc["id"], to_acc["id"], amount, "transfer", "user_transfer")
    write_counters(acc, txn + 1, loan)
    ok(balance=from_acc["balance"] - amount)

def action_loan_request(account_id_or_name, password, amount_s, reason):
    ensure_data_files()
    a = find_account(account_id_or_name)
    if not a or a["password"] != password:
        err("invalid_credentials"); return
    amount = float(amount_s)
    if amount <= 0:
        err("invalid_amount"); return
    acc, txn, loan = read_counters()
    ts = timestamp_str()
    reason_clean = reason.replace("|", " ").replace("\n", " ")[:500]
    with LOANS_FILE.open("a", encoding="utf-8") as f:
        f.write(f"{loan}|{a['id']}|{amount:.2f}|{reason_clean}|pending|{ts}\n")
    write_counters(acc, txn, loan + 1)
    ok(loan_id=loan)

def action_get_account(account_id_or_name, password):
    ensure_data_files()
    a = find_account(account_id_or_name)
    if not a or a["password"] != password:
        err("invalid_credentials"); return
    txns = []
    try:
        for line in TXNS_FILE.read_text(encoding="utf-8").splitlines():
            t = parse_txn_line(line)
            if t and (t["from"].lower() == a["id"].lower() or t["to"].lower() == a["id"].lower()):
                txns.append(t)
    except FileNotFoundError:
        pass
    ok(account_id=a["id"], name=a["name"], balance=a["balance"], role=a["role"], transactions=txns)

def action_get_loans_for(account_id_or_name, password):
    ensure_data_files()
    a = find_account(account_id_or_name)
    if not a or a["password"] != password:
        err("invalid_credentials"); return
    loans = []
    try:
        for line in LOANS_FILE.read_text(encoding="utf-8").splitlines():
            loan = parse_loan_line(line)
            if loan and loan["account_id"].lower() == a["id"].lower():
                loans.append({"loan_id": loan["loan_id"], "amount": loan["amount"],
                               "reason": loan["reason"], "status": loan["status"], "time": loan["time"]})
    except FileNotFoundError:
        pass
    ok(loans=loans)

def action_admin_accounts(admin_password):
    ensure_data_files(); ensure_admin()
    if not admin_check(admin_password):
        err("unauthorized"); return
    accounts = []
    try:
        for line in ACCOUNTS_FILE.read_text(encoding="utf-8").splitlines():
            a = parse_account_line(line)
            if a and a["role"] != "admin":
                accounts.append({"id": a["id"], "name": a["name"], "balance": a["balance"], "status": a["status"]})
    except FileNotFoundError:
        pass
    ok(accounts=accounts)

def action_admin_transactions(admin_password):
    ensure_data_files()
    if not admin_check(admin_password):
        err("unauthorized"); return
    txns = []
    try:
        for line in TXNS_FILE.read_text(encoding="utf-8").splitlines():
            t = parse_txn_line(line)
            if t:
                txns.append(t)
    except FileNotFoundError:
        pass
    ok(transactions=txns)

def action_admin_loans_pending(admin_password):
    ensure_data_files()
    if not admin_check(admin_password):
        err("unauthorized"); return
    loans = []
    try:
        for line in LOANS_FILE.read_text(encoding="utf-8").splitlines():
            loan = parse_loan_line(line)
            if loan and loan["status"] == "pending":
                loans.append(loan)
    except FileNotFoundError:
        pass
    ok(loans=loans)

def action_admin_loan_decision(admin_password, loan_id_s, decision):
    ensure_data_files()
    if not admin_check(admin_password):
        err("unauthorized"); return
    target = int(loan_id_s)
    new_status = "approved" if decision == "approve" else "rejected"
    try:
        lines = LOANS_FILE.read_text(encoding="utf-8").splitlines()
    except FileNotFoundError:
        err("no_loans"); return
    new_lines = []
    found_loan = None
    for line in lines:
        loan = parse_loan_line(line)
        if loan and loan["loan_id"] == target and loan["status"] == "pending":
            found_loan = loan
            new_lines.append(f"{loan['loan_id']}|{loan['account_id']}|{loan['amount']:.2f}|{loan['reason']}|{new_status}|{loan['time']}")
        else:
            new_lines.append(line)
    LOANS_FILE.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
    if not found_loan:
        err("loan_not_found_or_resolved"); return
    if decision == "approve":
        borrower = find_account(found_loan["account_id"])
        if borrower:
            update_account_balance(borrower["id"], borrower["balance"] + found_loan["amount"])
            acc, txn, loan_c = read_counters()
            append_transaction(txn, "LOAN", borrower["id"], found_loan["amount"], "loan_disbursement", "approved_loan")
            write_counters(acc, txn + 1, loan_c)
    ok(loan_id=target, decision=new_status)

DISPATCH = {
    ("signup",              5): lambda a: action_signup(a[2], a[3], a[4]),
    ("login",               4): lambda a: action_login(a[2], a[3]),
    ("deposit",             5): lambda a: action_deposit(a[2], a[3], a[4]),
    ("transfer",            6): lambda a: action_transfer(a[2], a[3], a[4], a[5]),
    ("loan_request",        6): lambda a: action_loan_request(a[2], a[3], a[4], a[5]),
    ("get_account",         4): lambda a: action_get_account(a[2], a[3]),
    ("get_loans_for",       4): lambda a: action_get_loans_for(a[2], a[3]),
    ("admin_accounts",      3): lambda a: action_admin_accounts(a[2]),
    ("admin_transactions",  3): lambda a: action_admin_transactions(a[2]),
    ("admin_loans_pending", 3): lambda a: action_admin_loans_pending(a[2]),
    ("admin_loan_decision", 5): lambda a: action_admin_loan_decision(a[2], a[3], a[4]),
}

if __name__ == "__main__":
    argv = sys.argv
    if len(argv) < 2:
        err("no_action")
        sys.exit(1)
    key = (argv[1], len(argv))
    handler = DISPATCH.get(key)
    if handler:
        handler(argv)
    else:
        err("unknown_action_or_bad_args")
        sys.exit(1)
