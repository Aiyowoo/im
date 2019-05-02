## 各个程序的用途

## dispatch_server
做负载均衡使用。客户端从dispatch_server请求可以连接的client_hub地址。_

### client_hub
客户端启动后与client_hub建立长连接，
来自客户端的消息将通过client_hub转发到message_server去处理，
同时将message_server的响应发送给正确的客户端。
每个client_hub都会将各自管理的连接发送给所有的message_server，
这样每个message_server都知道怎样将一条消息发送给客户端。

### message_server
真正处理来自客户端的请求。

### db_server
处理数据库相关的操作，有些数据不能并行处理（比如说用户发送消息的时间的确定）。