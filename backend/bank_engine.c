/*
 * bank_engine.c
 * -------------
 * Core banking logic for the mini-bank project.
 * Storage: plain .txt files (pipe-delimited records).
 * Invocation: ./bank_engine <action> <args...>
 * Output: a single line of JSON on stdout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define DATA_DIR "data"
#define ACCOUNTS_FILE DATA_DIR "/accounts.txt"
#define TXNS_FILE     DATA_DIR "/transactions.txt"
#define LOANS_FILE    DATA_DIR "/loans.txt"
#define COUNTERS_FILE DATA_DIR "/counters.txt"
#define LINE_MAX_LEN 2048

/* Case-insensitive string comparison helper */
static int str_ieq(const char *s1, const char *s2) {
    if (!s1 || !s2) return 0;
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
            return 0;
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

/* ---------- small helpers ---------- */

static void json_escape(const char *in, char *out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 2 < outsz; i++) {
        if (in[i] == '"' || in[i] == '\\') {
            out[j++] = '\\';
        }
        out[j++] = in[i];
    }
    out[j] = '\0';
}

static void ensure_data_files(void) {
    FILE *f;
    f = fopen(ACCOUNTS_FILE, "a"); if (f) fclose(f);
    f = fopen(TXNS_FILE, "a"); if (f) fclose(f);
    f = fopen(LOANS_FILE, "a"); if (f) fclose(f);
    f = fopen(COUNTERS_FILE, "r");
    if (!f) {
        f = fopen(COUNTERS_FILE, "w");
        fprintf(f, "1001|1|1\n"); /* next_account_id|next_txn_id|next_loan_id */
        fclose(f);
    } else {
        fclose(f);
    }
}

static void read_counters(long *acc, long *txn, long *loan) {
    FILE *f = fopen(COUNTERS_FILE, "r");
    *acc = 1001; *txn = 1; *loan = 1;
    if (f) {
        fscanf(f, "%ld|%ld|%ld", acc, txn, loan);
        fclose(f);
    }
}

static void write_counters(long acc, long txn, long loan) {
    FILE *f = fopen(COUNTERS_FILE, "w");
    if (f) { fprintf(f, "%ld|%ld|%ld\n", acc, txn, loan); fclose(f); }
}

static void timestamp_str(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tmv = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tmv);
}

/* ---------- account record ---------- */

typedef struct {
    char id[32];
    char name[128];
    char password[128];
    double balance;
    char status[16];   /* active / frozen */
    char role[16];      /* user / admin */
    int found;
} Account;

static Account find_account(const char *id_or_name) {
    Account a; memset(&a, 0, sizeof(a));
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) return a;
    char line[LINE_MAX_LEN];
    while (fgets(line, sizeof(line), f)) {
        char lid[32], name[128], pass[128], status[16], role[16];
        double bal;
        if (sscanf(line, "%31[^|]|%127[^|]|%127[^|]|%lf|%15[^|]|%15[^\n]",
                   lid, name, pass, &bal, status, role) == 6) {
            if (str_ieq(lid, id_or_name) || str_ieq(name, id_or_name)) {
                strcpy(a.id, lid); strcpy(a.name, name); strcpy(a.password, pass);
                a.balance = bal; strcpy(a.status, status); strcpy(a.role, role);
                a.found = 1;
                break;
            }
        }
    }
    fclose(f);
    return a;
}

/* Rewrites the accounts file updating a single account's balance/status */
static int update_account_balance(const char *id, double new_balance) {
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) return 0;
    char tmp_path[] = DATA_DIR "/accounts.tmp";
    FILE *out = fopen(tmp_path, "w");
    char line[LINE_MAX_LEN];
    int ok = 0;
    while (fgets(line, sizeof(line), f)) {
        char lid[32], name[128], pass[128], status[16], role[16];
        double bal;
        if (sscanf(line, "%31[^|]|%127[^|]|%127[^|]|%lf|%15[^|]|%15[^\n]",
                   lid, name, pass, &bal, status, role) == 6) {
            if (str_ieq(lid, id)) {
                fprintf(out, "%s|%s|%s|%.2f|%s|%s\n", lid, name, pass, new_balance, status, role);
                ok = 1;
                continue;
            }
        }
        fputs(line, out);
    }
    fclose(f); fclose(out);
    remove(ACCOUNTS_FILE);
    rename(tmp_path, ACCOUNTS_FILE);
    return ok;
}

static void append_account(const char *id, const char *name, const char *password,
                            double balance, const char *status, const char *role) {
    FILE *f = fopen(ACCOUNTS_FILE, "a");
    fprintf(f, "%s|%s|%s|%.2f|%s|%s\n", id, name, password, balance, status, role);
    fclose(f);
}

static void append_transaction(long txn_id, const char *from_id, const char *to_id,
                                double amount, const char *type, const char *note) {
    FILE *f = fopen(TXNS_FILE, "a");
    char ts[32]; timestamp_str(ts, sizeof(ts));
    fprintf(f, "%ld|%s|%s|%s|%.2f|%s|%s\n", txn_id, ts, from_id, to_id, amount, type, note);
    fclose(f);
}

/* ---------- ensure a seed admin account exists ---------- */
static void ensure_admin(void) {
    Account a = find_account("ADMIN");
    if (!a.found) {
        append_account("ADMIN", "Bank Administrator", "admin123", 0.0, "active", "admin");
    }
}

/* ---------- actions ---------- */

static void action_signup(const char *name, const char *password, const char *initial_deposit) {
    ensure_data_files(); ensure_admin();
    long acc, txn, loan;
    read_counters(&acc, &txn, &loan);
    char id[32];
    snprintf(id, sizeof(id), "ACC%ld", acc);
    double dep = atof(initial_deposit);
    if (dep < 0) dep = 0;
    append_account(id, name, password, dep, "active", "user");
    if (dep > 0) {
        append_transaction(txn, "EXTERNAL", id, dep, "deposit", "initial_deposit");
        txn++;
    }
    acc++;
    write_counters(acc, txn, loan);
    char esc_name[256]; json_escape(name, esc_name, sizeof(esc_name));
    printf("{\"status\":\"ok\",\"account_id\":\"%s\",\"name\":\"%s\",\"balance\":%.2f}\n", id, esc_name, dep);
}

static void action_login(const char *id_or_name, const char *password) {
    ensure_data_files(); ensure_admin();
    Account a = find_account(id_or_name);
    if (!a.found || strcmp(a.password, password) != 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_credentials\"}\n");
        return;
    }
    if (strcmp(a.status, "active") != 0) {
        printf("{\"status\":\"error\",\"message\":\"account_not_active\"}\n");
        return;
    }
    char esc_name[256]; json_escape(a.name, esc_name, sizeof(esc_name));
    printf("{\"status\":\"ok\",\"account_id\":\"%s\",\"name\":\"%s\",\"balance\":%.2f,\"role\":\"%s\"}\n",
           a.id, esc_name, a.balance, a.role);
}

static void action_deposit(const char *id_or_name, const char *password, const char *amount_s) {
    ensure_data_files();
    Account a = find_account(id_or_name);
    if (!a.found || strcmp(a.password, password) != 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_credentials\"}\n"); return;
    }
    double amount = atof(amount_s);
    if (amount <= 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_amount\"}\n"); return;
    }
    double new_bal = a.balance + amount;
    update_account_balance(a.id, new_bal);
    long acc, txn, loan; read_counters(&acc, &txn, &loan);
    append_transaction(txn, "EXTERNAL", a.id, amount, "deposit", "cash_deposit");
    write_counters(acc, txn + 1, loan);
    printf("{\"status\":\"ok\",\"balance\":%.2f}\n", new_bal);
}

static void action_transfer(const char *from_id_or_name, const char *password, const char *to_id_or_name, const char *amount_s) {
    ensure_data_files();
    Account from = find_account(from_id_or_name);
    if (!from.found || strcmp(from.password, password) != 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_credentials\"}\n"); return;
    }
    Account to = find_account(to_id_or_name);
    if (!to.found) {
        printf("{\"status\":\"error\",\"message\":\"recipient_not_found\"}\n"); return;
    }
    double amount = atof(amount_s);
    if (amount <= 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_amount\"}\n"); return;
    }
    if (from.balance < amount) {
        printf("{\"status\":\"error\",\"message\":\"insufficient_funds\"}\n"); return;
    }
    if (str_ieq(from.id, to.id)) {
        printf("{\"status\":\"error\",\"message\":\"cannot_transfer_to_self\"}\n"); return;
    }
    update_account_balance(from.id, from.balance - amount);
    update_account_balance(to.id, to.balance + amount);
    long acc, txn, loan; read_counters(&acc, &txn, &loan);
    append_transaction(txn, from.id, to.id, amount, "transfer", "user_transfer");
    write_counters(acc, txn + 1, loan);
    printf("{\"status\":\"ok\",\"balance\":%.2f}\n", from.balance - amount);
}

static void action_loan_request(const char *id_or_name, const char *password, const char *amount_s, const char *reason) {
    ensure_data_files();
    Account a = find_account(id_or_name);
    if (!a.found || strcmp(a.password, password) != 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_credentials\"}\n"); return;
    }
    double amount = atof(amount_s);
    if (amount <= 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_amount\"}\n"); return;
    }
    long acc, txn, loan; read_counters(&acc, &txn, &loan);
    char ts[32]; timestamp_str(ts, sizeof(ts));
    FILE *f = fopen(LOANS_FILE, "a");
    char reason_clean[512]; strncpy(reason_clean, reason, sizeof(reason_clean)-1);
    reason_clean[sizeof(reason_clean)-1] = 0;
    for (char *p = reason_clean; *p; p++) if (*p == '|' || *p == '\n') *p = ' ';
    fprintf(f, "%ld|%s|%.2f|%s|pending|%s\n", loan, a.id, amount, reason_clean, ts);
    fclose(f);
    write_counters(acc, txn, loan + 1);
    printf("{\"status\":\"ok\",\"loan_id\":%ld}\n", loan);
}

static void action_get_account(const char *id_or_name, const char *password) {
    ensure_data_files();
    Account a = find_account(id_or_name);
    if (!a.found || strcmp(a.password, password) != 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_credentials\"}\n"); return;
    }
    char esc_name[256]; json_escape(a.name, esc_name, sizeof(esc_name));
    printf("{\"status\":\"ok\",\"account_id\":\"%s\",\"name\":\"%s\",\"balance\":%.2f,\"role\":\"%s\",\"transactions\":[",
           a.id, esc_name, a.balance, a.role);
    FILE *f = fopen(TXNS_FILE, "r");
    char line[LINE_MAX_LEN];
    int first = 1;
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            long tid; char ts[32], from_id[32], to_id[32], type[16], note[256];
            double amt;
            if (sscanf(line, "%ld|%31[^|]|%31[^|]|%31[^|]|%lf|%15[^|]|%255[^\n]",
                       &tid, ts, from_id, to_id, &amt, type, note) == 7) {
                if (str_ieq(from_id, a.id) || str_ieq(to_id, a.id)) {
                    if (!first) printf(",");
                    first = 0;
                    printf("{\"id\":%ld,\"time\":\"%s\",\"from\":\"%s\",\"to\":\"%s\",\"amount\":%.2f,\"type\":\"%s\",\"note\":\"%s\"}",
                           tid, ts, from_id, to_id, amt, type, note);
                }
            }
        }
        fclose(f);
    }
    printf("]}\n");
}

static int admin_check(const char *password) {
    Account a = find_account("ADMIN");
    return a.found && strcmp(a.password, password) == 0;
}

static void action_admin_accounts(const char *admin_password) {
    ensure_data_files(); ensure_admin();
    if (!admin_check(admin_password)) { printf("{\"status\":\"error\",\"message\":\"unauthorized\"}\n"); return; }
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    printf("{\"status\":\"ok\",\"accounts\":[");
    char line[LINE_MAX_LEN]; int first = 1;
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            char lid[32], name[128], pass[128], status[16], role[16]; double bal;
            if (sscanf(line, "%31[^|]|%127[^|]|%127[^|]|%lf|%15[^|]|%15[^\n]",
                       lid, name, pass, &bal, status, role) == 6) {
                if (str_ieq(role, "admin")) continue;
                char esc_name[256]; json_escape(name, esc_name, sizeof(esc_name));
                if (!first) printf(",");
                first = 0;
                printf("{\"id\":\"%s\",\"name\":\"%s\",\"balance\":%.2f,\"status\":\"%s\"}",
                       lid, esc_name, bal, status);
            }
        }
        fclose(f);
    }
    printf("]}\n");
}

static void action_admin_transactions(const char *admin_password) {
    ensure_data_files();
    if (!admin_check(admin_password)) { printf("{\"status\":\"error\",\"message\":\"unauthorized\"}\n"); return; }
    FILE *f = fopen(TXNS_FILE, "r");
    printf("{\"status\":\"ok\",\"transactions\":[");
    char line[LINE_MAX_LEN]; int first = 1;
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            long tid; char ts[32], from_id[32], to_id[32], type[16], note[256]; double amt;
            if (sscanf(line, "%ld|%31[^|]|%31[^|]|%31[^|]|%lf|%15[^|]|%255[^\n]",
                       &tid, ts, from_id, to_id, &amt, type, note) == 7) {
                if (!first) printf(",");
                first = 0;
                printf("{\"id\":%ld,\"time\":\"%s\",\"from\":\"%s\",\"to\":\"%s\",\"amount\":%.2f,\"type\":\"%s\",\"note\":\"%s\"}",
                       tid, ts, from_id, to_id, amt, type, note);
            }
        }
        fclose(f);
    }
    printf("]}\n");
}

static void action_admin_loans_pending(const char *admin_password) {
    ensure_data_files();
    if (!admin_check(admin_password)) { printf("{\"status\":\"error\",\"message\":\"unauthorized\"}\n"); return; }
    FILE *f = fopen(LOANS_FILE, "r");
    printf("{\"status\":\"ok\",\"loans\":[");
    char line[LINE_MAX_LEN]; int first = 1;
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            long lid; char acc_id[32], status[16], reason[512], ts[32]; double amt;
            if (sscanf(line, "%ld|%31[^|]|%lf|%511[^|]|%15[^|]|%31[^\n]",
                       &lid, acc_id, &amt, reason, status, ts) == 6) {
                if (strcmp(status, "pending") != 0) continue;
                char esc_reason[1024]; json_escape(reason, esc_reason, sizeof(esc_reason));
                if (!first) printf(",");
                first = 0;
                printf("{\"loan_id\":%ld,\"account_id\":\"%s\",\"amount\":%.2f,\"reason\":\"%s\",\"time\":\"%s\"}",
                       lid, acc_id, amt, esc_reason, ts);
            }
        }
        fclose(f);
    }
    printf("]}\n");
}

static void action_admin_loan_decision(const char *admin_password, const char *loan_id_s, const char *decision) {
    ensure_data_files();
    if (!admin_check(admin_password)) { printf("{\"status\":\"error\",\"message\":\"unauthorized\"}\n"); return; }
    long target = atol(loan_id_s);
    FILE *f = fopen(LOANS_FILE, "r");
    if (!f) { printf("{\"status\":\"error\",\"message\":\"no_loans\"}\n"); return; }
    char tmp_path[] = DATA_DIR "/loans.tmp";
    FILE *out = fopen(tmp_path, "w");
    char line[LINE_MAX_LEN];
    int found = 0; char borrower[32] = ""; double loan_amount = 0;
    const char *new_status = (strcmp(decision, "approve") == 0) ? "approved" : "rejected";
    while (fgets(line, sizeof(line), f)) {
        long lid; char acc_id[32], status[16], reason[512], ts[32]; double amt;
        if (sscanf(line, "%ld|%31[^|]|%lf|%511[^|]|%15[^|]|%31[^\n]",
                   &lid, acc_id, &amt, reason, status, ts) == 6) {
            if (lid == target && strcmp(status, "pending") == 0) {
                fprintf(out, "%ld|%s|%.2f|%s|%s|%s\n", lid, acc_id, amt, reason, new_status, ts);
                found = 1; strcpy(borrower, acc_id); loan_amount = amt;
                continue;
            }
        }
        fputs(line, out);
    }
    fclose(f); fclose(out);
    remove(LOANS_FILE); rename(tmp_path, LOANS_FILE);
    if (!found) { printf("{\"status\":\"error\",\"message\":\"loan_not_found_or_resolved\"}\n"); return; }
    if (strcmp(decision, "approve") == 0) {
        Account b = find_account(borrower);
        if (b.found) {
            update_account_balance(b.id, b.balance + loan_amount);
            long acc, txn, loan; read_counters(&acc, &txn, &loan);
            append_transaction(txn, "LOAN", b.id, loan_amount, "loan_disbursement", "approved_loan");
            write_counters(acc, txn + 1, loan);
        }
    }
    printf("{\"status\":\"ok\",\"loan_id\":%ld,\"decision\":\"%s\"}\n", target, new_status);
}

static void action_get_loans_for(const char *id_or_name, const char *password) {
    ensure_data_files();
    Account a = find_account(id_or_name);
    if (!a.found || strcmp(a.password, password) != 0) {
        printf("{\"status\":\"error\",\"message\":\"invalid_credentials\"}\n"); return;
    }
    FILE *f = fopen(LOANS_FILE, "r");
    printf("{\"status\":\"ok\",\"loans\":[");
    char line[LINE_MAX_LEN]; int first = 1;
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            long lid; char acc_id[32], status[16], reason[512], ts[32]; double amt;
            if (sscanf(line, "%ld|%31[^|]|%lf|%511[^|]|%15[^|]|%31[^\n]",
                       &lid, acc_id, &amt, reason, status, ts) == 6) {
                if (!str_ieq(acc_id, a.id)) continue;
                char esc_reason[1024]; json_escape(reason, esc_reason, sizeof(esc_reason));
                if (!first) printf(",");
                first = 0;
                printf("{\"loan_id\":%ld,\"amount\":%.2f,\"reason\":\"%s\",\"status\":\"%s\",\"time\":\"%s\"}",
                       lid, amt, esc_reason, status, ts);
            }
        }
        fclose(f);
    }
    printf("]}\n");
}

/* ---------- main dispatcher ---------- */

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("{\"status\":\"error\",\"message\":\"no_action\"}\n");
        return 1;
    }
    const char *action = argv[1];

    if (strcmp(action, "signup") == 0 && argc == 5) {
        action_signup(argv[2], argv[3], argv[4]);
    } else if (strcmp(action, "login") == 0 && argc == 4) {
        action_login(argv[2], argv[3]);
    } else if (strcmp(action, "deposit") == 0 && argc == 5) {
        action_deposit(argv[2], argv[3], argv[4]);
    } else if (strcmp(action, "transfer") == 0 && argc == 6) {
        action_transfer(argv[2], argv[3], argv[4], argv[5]);
    } else if (strcmp(action, "loan_request") == 0 && argc == 6) {
        action_loan_request(argv[2], argv[3], argv[4], argv[5]);
    } else if (strcmp(action, "get_account") == 0 && argc == 4) {
        action_get_account(argv[2], argv[3]);
    } else if (strcmp(action, "get_loans_for") == 0 && argc == 4) {
        action_get_loans_for(argv[2], argv[3]);
    } else if (strcmp(action, "admin_accounts") == 0 && argc == 3) {
        action_admin_accounts(argv[2]);
    } else if (strcmp(action, "admin_transactions") == 0 && argc == 3) {
        action_admin_transactions(argv[2]);
    } else if (strcmp(action, "admin_loans_pending") == 0 && argc == 3) {
        action_admin_loans_pending(argv[2]);
    } else if (strcmp(action, "admin_loan_decision") == 0 && argc == 5) {
        action_admin_loan_decision(argv[2], argv[3], argv[4]);
    } else {
        printf("{\"status\":\"error\",\"message\":\"unknown_action_or_bad_args\"}\n");
        return 1;
    }
    return 0;
}
