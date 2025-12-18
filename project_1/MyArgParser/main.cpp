#include <iostream>
#include "Gemini_Monitor.hpp"

int main(int argc, char* argv[]){
    ArgParse parser;

    // 定义期望参数
    parser.add_argument("help", "h", "Show help message", false);
    parser.add_argument("version", "v", "Show version", false);
    parser.add_argument("port", "p", "Port to listen on", true, "8080");
    parser.add_argument("config", "c", "Configuration file path", true);
    parser.add_argument("verbose", "V", "Enable verbose logging", false);

    try {
        // 解析命令行
        parser.parse(argc, argv);
        // 处理逻辑
        if(parser.has("help")){
            parser.print_help();
            return 0;
        }
        if(parser.has("version")){
            std::cout << "CLI Tool v1.0.0" << std::endl;
            return 0;
        }
        // 获取参数值
        int port = parser.get<int>("port");
        bool verbose = parser.get<bool>("verbose");

        std::cout << "Starting server..." << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Verbose Mode: " << (verbose ? "NO" : "OFF") << std::endl;

        if(parser.has("config")){
            std::string config_path = parser.get<std::string>("config");
            std::cout << "Loading config from: " << config_path << std::endl;
        }else{
            std::cout << "No config file specified." << std::endl;
        }

    } catch (const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
        parser.print_help();
        return 1;
    }
    return 0;
}