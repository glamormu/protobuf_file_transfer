'''
start server
get port
kill server or wait it stop
get state of file transfering
'''
import os
import time
write_path = "server_in.pipe" 
read_path = "server_out.pipe"
def start_server(token, dir_str):
    try:
        os.mkfifo( write_path )
        os.mkfifo( read_path )
    except OSError:
        print ("mkfifo error")

    pid = os.fork()
    if pid == 0:
        os.execl("zuolin_file_server","zuolin_file_server",
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
            port = int(s)
            print(port)
            break
        print(os.wait())

if __name__ == '__main__':
    start_server("cv", "/home/zhang/")

