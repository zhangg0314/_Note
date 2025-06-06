# ——Docker Compose——

# 背景需求

1. **多容器管理的需求**
   在现代软件开发中，尤其是采用微服务架构的应用系统，一个应用通常由多个微服务组成，每个微服务可能需要多个实例。例如，一个典型的 Web 应用可能包括前端服务、后端服务、数据库服务、缓存服务等多个组件。如果每个服务都需要手动启动、停止和管理，那么效率会非常低下，维护成本也会显著增加。
2. **简化容器编排**
   Docker 提供了强大的容器化功能，但最初主要集中在单个容器的管理和操作上。虽然可以通过` docker build` 和` docker run `等命令手动管理多个容器，但这种方式在处理复杂的多容器应用时显得非常繁琐。
3. **Fig 的启发**
   Docker Compose 的前身是 Fig，这是一个由 Orchard 公司开发的工具，用于管理基于 Docker 的多容器应用。Fig 允许用户通过一个 YAML 文件定义多容器应用，并使用命令行工具进行部署和管理。Fig 的出现为 Docker Compose 提供了灵感和基础。

【参考链接】[Docker入门之docker-compose - 心冰之海 - 博客园](https://www.cnblogs.com/lhxsoft/p/18643817)