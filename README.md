# ComputerNetworkLabs

## 项目简介

ComputerNetworkLabs 是计算机网络课程的实验与作业归档仓库，覆盖了从协议分析、二层交换、静态路由、OSPF，到 Socket 编程和简易 Web 服务器实现的一整套学习路径。

仓库同时包含实验文档、截图、抓包、代码和可执行文件，目标是完整保留“实验设计 -> 实施过程 -> 结果验证”的全过程资料，便于后续复盘与课程展示。

## 项目架构

项目按照课程实验主题分层组织：

- 网络原理与设备配置层：`lab1` ~ `lab4`
- 网络编程实践层：`lab5`（Socket 客户端/服务端）
- 应用协议实践层：`lab6`（HTTP Web Server）
- 实验结果归档层：`作业` 与各 lab 中的文档/截图/附件

其中可运行代码的逻辑架构如下：

- Socket 实验：客户端发起连接与命令，服务端解析数据包并响应
- Web 实验：服务端监听端口，解析 HTTP 请求并返回静态资源或登录结果

## 目录说明

- `lab1`: 协议分析实验资料（含抓包文件）
- `lab2`: 二层交换网络组网实验资料
- `lab3`: 静态路由配置实验资料
- `lab4`: OSPF 相关实验资料
- `lab5`: 基于 Socket 的自定义协议通信实验（含 C++ 客户端/服务端）
- `lab6`: 轻量级 Web Server 实验（含 C++ 实现与静态资源）
- `作业`: 课程习题与作业文档

## 可运行代码

### Lab5 Socket 通信

代码位置：`lab5/lab7-socket`

```bash
g++ server.cpp -o server.exe -lws2_32
g++ client.cpp -o client.exe -lws2_32
```

先运行 `server.exe`，再运行 `client.exe`，按菜单进行连接、查询和消息发送。

### Lab6 轻量级 Web Server

代码位置：`lab6/lab6-web/web/sever.cpp`

```bash
g++ sever.cpp -o sever.exe -lws2_32
```

启动后默认监听 `2642` 端口。静态资源位于 `lab6/lab6-web/info/`，可自行构造 `GET/POST` 请求进行验证。

## 环境说明

- 代码主要基于 Windows + WinSock2 开发。
- 仓库中包含较多实验附件（`.doc/.docx/.pdf/.zip/.png`），用于课程归档与复现实验过程。
