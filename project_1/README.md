## 简单的命令行解析器

### what

实现一个处理命令行参数的工具，支持短选项、长选项和参数值

### how

```
# 编译 (指定 C++17 标准)
g++ -std=c++17 main.cpp -o arg_test

# 运行测试 1：查看帮助
./arg_test --help

# 运行测试 2：混合使用长短参数
./arg_test -p 9000 --config=/tmp/test.conf -d

# 运行测试 3：触发错误（漏掉参数值）
./arg_test --config
```

### learn

1. 学会从命令行获取参数
2. 加深了对迭代器的使用
3. 没有使用命名空间