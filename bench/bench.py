import socket
import threading
import time
import sys
import json

# 服务器信息
SERVER_IP = '127.0.0.1'  # 服务器 IP 地址
SERVER_PORT = 9000         # 服务器端口

# 每个线程的目标函数
def send_request(thread_id):
    try:
        # 创建 socket 对象
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # 连接到服务器
        sock.connect((SERVER_IP, SERVER_PORT))
        
        # 发送数据
        message = {
            "msgid": 3,
            "username": "zhang san",
            "password": "178"
        }

        # 将消息转换为 JSON 字符串
        json_message = json.dumps(message)

        # 发送数据
        sock.sendall(json_message.encode('utf-8'))
        
        # 接收服务器响应
        response = sock.recv(1024)
        # print(f'Thread {thread_id} received: {response.decode()}')
        
    except Exception as e:
        print(f'Thread {thread_id} encountered an error: {e}')
    finally:
        sock.close()

# 创建和启动多个线程
def run_load_test(num_threads):
    threads = []
    for i in range(num_threads):
        thread = threading.Thread(target=send_request, args=(i,))
        threads.append(thread)
        thread.start()
    
    # 等待所有线程完成
    for thread in threads:
        thread.join()

if __name__ == '__main__':
    num_threads = 100  # 默认线程数
    if len(sys.argv): # 如果有参数
        num_threads = int(sys.argv[1])
    print(f'Starting load test with {num_threads} threads...')
    start_time = time.time()
    run_load_test(num_threads)
    end_time = time.time()
    
    print(f'Load test completed in {end_time - start_time:.2f} seconds.')