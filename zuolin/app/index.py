import json
from flask import jsonify
from flask import request
from flask import redirect
from flask import render_template
from app import app
import os
import time
import subprocess
import sqlite3
from itsdangerous import TimedJSONWebSignatureSerializer as Serializer
import itsdangerous

SECRET_KEY = 'I love you more than I can say'
users = list()
#PRO_ROOT_PATH=os.getenv('ZUOLIN_PRO_PATH')
PRO_ROOT_PATH='/home/vergil/zuolin'

print(PRO_ROOT_PATH)
FILE_ROOT_PATH = PRO_ROOT_PATH+'/www/'
HOME_PATH=os.getenv('HOME')

@app.route('/')
def index():
    return "Welcome!"

@app.route('/testjson', methods=['GET', 'POST'])
def testjson():
    if request.method == 'POST':
        a = request.get_data()
        dict1 = json.loads(a.decode('utf-8'))
        return json.dumps(dict1["data"])
    else:
        return '<h1>post only</h1>'

def verify_user(user_dict):
    conn = sqlite3.connect('zuolin.db')
    cursor = conn.cursor()
    cursor.execute('select * from users where name = ? and password = ?',
        (user_dict['username'], user_dict['password']))
    values = cursor.fetchall()
    if (len(values) == 1):
        return True
    elif (len(values) == 0):
        return False
    else: 
        return False

def generate_auth_token(username):
    s = Serializer(SECRET_KEY)
    return s.dumps({'username':username})

def verify_auth_token(username, token):
    s = Serializer(SECRET_KEY)
    try:
        data = s.loads(token)
    except itsdangerous.SignatureExpired:
        return False
    except itsdangerous.BadSignature:
        return False
    return True

@app.route('/logout', methods=['POST'])
def logout(): 
    req_data = request.get_data()
    req_dict = json.loads(req_data.decode('utf-8'))
    username = req_dict['username']
    token = req_dict['token']
    ret_dict = {}
    if (username not in users):
        ret_dict['error'] = 'not logged in'
        return jsonify(ret_dict)
    if (verify_auth_token(username, token)):
        ret_dict['error'] = ''
        ret_dict['data'] = 'logged out'
        users.remove(username)
        return jsonify(ret_dict)
    else:
        ret_dict['error'] = 'invalid'
        return jsonify(ret_dict)

@app.route('/login', methods=['POST'])
def login():
    ret_dict = {}
    req_data = request.get_data()
    req_dict = json.loads(req_data.decode('utf-8'))
    username = req_dict['username']
    if (verify_user(req_dict)):
        ret_dict['data'] = 'ok'
        ret_dict['error'] = 0
        ret_dict['token'] = generate_auth_token(username).decode('ascii')
        if (username in users):#already logged in
            ret_dict['data'] = 'already logged in'
        else:
            users.append(username)
        return jsonify(ret_dict)
    else:
        ret_dict['error'] = 1 #AUTH_FAILED
        return jsonify(ret_dict)

@app.route('/files', methods=['POST'])
def files():
    ret_dict = {}
    req_data = request.get_data()
    req_dict = json.loads(req_data.decode('utf-8'))
    username = req_dict['username']
    token = req_dict['token']
    if (username not in users):
        ret_dict['error'] = 'not logged in'
        return jsonify(ret_dict)
    if (verify_auth_token(username, token)):
        return jsonify(handle_command(req_dict))
    else:
        ret_dict['error'] = 'invalid'
        return jsonify(ret_dict)

def handle_command(req_dict):
    command = req_dict['command']
    if command == 'ls':
        return file_ls(req_dict)
    elif command == 'upload_file':
        return file_upload(req_dict)
    elif command == 'download_file':
        return file_download(req_dict)
    elif command == 'mv_file':
        return file_mv(req_dict)
    elif command == 'rm_file':
        return file_rm(req_dict)
    elif command == 'history':
        return history(req_dict)
    else:
        ret_dict = {}
        ret_dict['error'] = -1
        req_dict['err_msg'] = 'Unknow command'
        return req_dict

def history(req_dict):
    data_dict = {}
    data_dict['error'] = 0
    return data_dict

def file_rm(req_dict):
    data_dict = {}
    data_dict['error'] = 0
    return data_dict

def file_mv(req_dict):
    data_dict = {}
    data_dict['error'] = 0
    return data_dict

def file_download(req_dict):
    data_dict = {}
    file_path = req_dict['file']
    data_dict['error'] = 0
    return data_dict

def file_ls(req_dict):
    dir = req_dict['dir']
    data_dict = {}
    #check dir.
    if('..' in dir):
        data_dict['error'] = 1
        data_dict['err_msg'] = '.. in dir'
        return data_dict
    #ls

    path = FILE_ROOT_PATH+dir
    ls_res = []
    for dirpath, dirnames, filenames in os.walk(path):
        for file in filenames:
            fullpath = os.path.join(dirpath, file)
            ls_res.append(fullpath)
    data_dict['data'] = ls_res
    data_dict['error'] = 0
    return data_dict
    
write_path = "server_in.pipe" 
read_path = "server_out.pipe"
def start_server(token, dir_str):
    try:
        os.mkfifo( write_path )
        os.mkfifo( read_path )
    except OSError:
        print ("mkfifo error")
    server_port = 0
    pid = os.fork()
    if pid == 0:
        os.execl(PRO_ROOT_PATH + "/app/file_transfer/server/zuolin_file_server","zuolin_file_server",
                "-t"+token, "-d "+dir_str)
    else:
        read_path = str(pid)+".pipe"
        timeout = 3
        while timeout > 0:
            if os.path.exists(read_path) == False:
                #retry
                time.sleep(1)
                timeout -= 1
            else:
                break
        if timeout == 0:
            print("error")
        pipe_in = os.open(read_path, os.O_RDONLY )

        while True:
            s = os.read(pipe_in, 10)
            if len(s) == 0:
                time.sleep(1)
                continue
            server_port = int(s)
            break
        return server_port

def file_upload(req_dict):
    data_dict = {}
    data_dict['error'] = 0
    return data_dict
'''    
    username = req_dict['username']
    token = req_dict['token']
    # start file server with token and username
    server_port = start_server(token, HOME_PATH)
    data_dict = {}
    data_dict['err'] = 0
    data_dict['port'] = server_port
    return data_dict
'''

def main():
    index()
  
if __name__ == "__main__":
    main()
