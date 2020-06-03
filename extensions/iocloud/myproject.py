from login import create_app

'''
from flask import Flask, Response
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager
import time

db = SQLAlchemy()
login_manager = LoginManager()

def create_app():
    app = Flask(__name__, instance_relative_config=False)
    app.config.from_object('config.Config')

    # Initialize Plugins
    db.init_app(app)
    login_manager.init_app(app)

    with app.app_context():
        from login import routes
        from login import auth
        from login.assets import compile_static_assets

        # Register Blueprints
        app.register_blueprint(routes.main_bp)
        app.register_blueprint(auth.auth_bp)

        # Create Database Models
        db.create_all()

        # Compile static assets
        if app.config['FLASK_ENV'] == 'development':
            compile_static_assets(app)

        return app            

    @app.route("/")
    def hello():
        return "<h1 style='color:blue'>See Nixon <a href=\"video_feed\">link text</a> !</h1>"

    @app.route('/video_feed')
    def video_feed():
        """Video streaming route. Put this in the src attribute of an img tag."""
        return Response(gen(),
                        mimetype='multipart/x-mixed-replace; boundary=frame')
    def gen():
        """Video streaming generator function."""
        i = 0
        while True:
            path = "files/IMG" + str(i) + ".jpg"
            image = open(path, 'rb').read()
            i = i + 1
            if i > 3:
                i = 0

            yield (b'--frame\r\n'
                b'Content-Type: image/jpeg\r\n\r\n' + image + b'\r\n')

            time.sleep(0.2)
'''

if __name__ == "__main__":
    app = create_app()
    app.run(host='0.0.0.0', debug=True)
