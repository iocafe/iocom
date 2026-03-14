# from myproject import app
# if __name__ == "__main__":
#     app.run()
    
import login 
app = login.create_app()

if __name__ == "__main__":
    app.run(host='0.0.0.0', debug=True)
