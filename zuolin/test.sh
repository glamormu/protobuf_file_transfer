echo 'test json'
curl -H "Content-Type: application/json" -X POST  --data '{"opr": "add","data":
{ "userName": "98997","disc": "hudihiudhu", "expDate":"2", "ip":
["10.10.11.1"]}}' https://cvzhang.cn/testjson

echo -e '\ntest login\n'
curl -H "Content-Type: application/json" -X POST  --data '{"username":"vergil","password":"vergil"}' https://cvzhang.cn/login
curl -H "Content-Type: application/json" -X POST  --data '{"username":"vergil","password":"vergil2"}' https://cvzhang.cn/login
echo -e '\ntest files ls -------\n'
curl -H "Content-Type: application/json" -X POST  --data '{"username":"vergil","token":"eyJhbGciOiJIUzI1NiIsImlhdCI6MTUyNTU5Mjk4MywiZXhwIjoxNTI1NTk2NTgzfQ.eyJ1c2VybmFtZSI6InZlcmdpbCJ9.6PCAX9d2cGosJixr0DoJperoJJ0632A1aDuHIvlX7ZY","command":"ls","dir":"zhang"}' https://cvzhang.cn/files

echo -e '\ntest files ls -------\n'
curl -H "Content-Type: application/json" -X POST  --data '{"username":"vergil","token":"eyJleHAiOjE1MjM1MjMwMzgsImFsZyI6IkhTMjU2IiwiaWF0IjoxNTIzNTE5NDM4fQ.eyJ1c2VybmFtZSI6InZlcmdpbCJ9.P4pboC2OuR6xNbrelAMaNHuoQbOyUqwU1lCAqNmnkOc","command":"upload_file","dir":"zhang"}' https://cvzhang.cn/files
