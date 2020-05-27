from flask import Flask, Response
import time

app = Flask(__name__)

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


if __name__ == "__main__":
    app.run(host='0.0.0.0')
