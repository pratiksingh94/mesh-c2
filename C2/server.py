from dotenv import load_dotenv
import os

load_dotenv()

from db_schema import init_db

from flask import Flask # type: ignore
from routes.implants import implants_blueprint
from routes.admin import admin_blueprint



app = Flask(__name__)

def create_app():
    init_db()

    # health check
    @app.route('/')
    def index():
        return "OK", 200

    app.register_blueprint(implants_blueprint)
    app.register_blueprint(admin_blueprint)
    return app


if __name__ == "__main__":
    create_app().run(host="0.0.0.0", port=os.environ.get("PORT"), debug=os.environ.get("DEBUG", "false").lower() == "true")
