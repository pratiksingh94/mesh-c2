import sqlite3

# SQLite stuff setup

def init_db():
    """init the database schema"""
    conn = sqlite3.connect("c2.db")
    conn.execute("PRAGMA foreign_keys = ON;")
    c = conn.cursor()

    c.execute("""
    CREATE TABLE IF NOT EXISTS implants (
        id             INTEGER PRIMARY KEY,
        hostname       TEXT,
        ip             TEXT    NOT NULL,
        port           INTEGER NOT NULL,
        last_heartbeat DATETIME DEFAULT CURRENT_TIMESTAMP,
        registered_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
        UNIQUE(ip, port)     -- composite UNIQUE!
    );
    """)

    c.execute("""
    CREATE TABLE IF NOT EXISTS commands (
        id            INTEGER PRIMARY KEY AUTOINCREMENT,
        command_text  TEXT    NOT NULL,
        sent_at       DATETIME DEFAULT CURRENT_TIMESTAMP,
        status        TEXT DEFAULT 'pending',
        target        TEXT DEFAULT 'all'
    );
    """)

    c.execute("""
    CREATE TABLE IF NOT EXISTS results (
        id            INTEGER PRIMARY KEY AUTOINCREMENT,
        command_id    INTEGER NOT NULL,
        implant_id    INTEGER NOT NULL,
        output        TEXT,
        received_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(command_id) REFERENCES commands(id),
        FOREIGN KEY(implant_id) REFERENCES implants(id),
        UNIQUE(command_id, implant_id)
    );
    """)

    conn.commit()
    conn.close()
