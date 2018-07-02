from flask import Flask
 
app = Flask(__name__)
# app.config['BASIC_AUTH_FORCE'] = True
from app import index

if __name__ == '__main__':
	app.run()
