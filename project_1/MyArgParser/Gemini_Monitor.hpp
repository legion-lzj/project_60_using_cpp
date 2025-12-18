#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <type_traits>

class ArgParse{
public:
    struct Argument {
        std::string long_name;  // --output
        std::string short_name; // -o
        std::string description;    // 帮助信息
        bool requires_value;        // 是否需要后续参数
        std::string default_value;  // 默认值
    };

private:
    // 储存注册参数
    std::vector<Argument> known_args;
    // 存储解析后的键值对
    std::unordered_map<std::string, std::string> parsed_args;
    
public:
    // 1. 注册参数
    void add_argument(const std::string& long_name, 
                        const std::string& short_name,
                        const std::string& description,
                        bool requires_value = true,
                        const std::string& default_value = "")
    {
        known_args.push_back({long_name, short_name, description, requires_value, default_value});
    }

    // 2. 核心逻辑解析
    /* 
        读取命令行参数，分开长选项还是短选项 ,  
        长选项 (例如 --port 或 --config=test.ini)
        短选项 ( -o)
    */
   void parse(int argc, char* argv[]){
        // argv[0] —— 程序名字
        for(int i = 1; i < argc; i ++){
            std::string current_arg = argv[i];
            // long
            if(current_arg.substr(0, 2) == "--"){
                std::string key = current_arg.substr(2);
                std::string value;
                // 找到返回 index，不然是 npos
                size_t eq_pos = current_arg.find('=');
                // --config=test.ini
                if(eq_pos != std::string::npos){
                    value = key.substr(eq_pos + 1);
                    key = key.substr(0, eq_pos);
                    store_value(key, value);
                }else{
                    // 查找 key 定义，来确定有没有后续参数
                    auto it = find_args_by_long(key);
                    if(it != known_args.end()){
                        if(it->requires_value){
                            // 取下一个 argv
                            if(i + 1 < argc && argv[i + 1][0] != '-'){
                                store_value(key, argv[++i]);
                            }else
                                throw std::runtime_error("Option --" + key + " require a value.");
                        }else{
                            // 这是一个开关值，不需要参数
                            store_value(key, "true");
                        }
                    }else{
                        std::cerr << "Warning: Unknown option -- " << key << std::endl;
                    }
                }
            }
            // 如果是短选项，统一存储为长选项
            else if(current_arg.substr(0, 1) == "-") {
                std::string key = current_arg.substr(1);
                std::string value;
                auto it = find_args_by_short(key);
                if(it != known_args.end()){
                    if(it->requires_value){
                        if(i + 1 < argc && argv[i+1][0] != '-'){
                            store_value(it->long_name, argv[++i]);
                        }else{
                                throw std::runtime_error("Option -" + key + " require a value.");
                        }
                    }else{
                        store_value(it->long_name, "true");
                    }
                }
            }
        }
   }

   // 3. 获取值的模板函数
   template <typename T>
   T get(const std::string& key) const{
        // 先看解析结果里面有没有
        if(parsed_args.find(key) != parsed_args.end()){
            return convert<T> (parsed_args.at(key));
        }
        // 没有的话，查一下是否有默认值
        auto it = find_args_by_long(key);
        if(it != known_args.end() && it->default_value.empty()){
            return convert<T> (it->default_value);
        }
        // 两者都没有，对于 bool 返回 false，其他抛出异常或返回默认构造
        // std::is_same<T, bool> 本身是一个结构体类型。 
        // 这个结构体里有一个静态常量成员叫 value，它存的才是那个布尔值 true 或 false。
        if constexpr(std::is_same<T, bool>::value)  return false;
        throw std::runtime_error("Argument no found: " + key);
   }
   
   // 检查某个参数是否存在
   bool has(const std::string& key) {
        return parsed_args.find(key) != parsed_args.end(); 
   }

   // 4. 生成帮助信息
   void print_help() const{
        std::cout << "Usage: " << std::endl;
        for(auto& arg : known_args){
            std::cout << "  -" << arg.short_name << ", --" << arg.long_name;
            if (arg.requires_value) std::cout << " <value>";
            std::cout << "\t" << arg.description;
            if (!arg.default_value.empty()) std::cout << " (default: " << arg.default_value << ")";
            std::cout << std::endl;
        }
   }

// 辅助参数
private:
    // 根据长名查找命令 --- find_if —— 有条件的 find
    /* lambada表达式里面传入的arg是known_args的每一个成员 */
    std::vector<Argument>::const_iterator find_args_by_long (const std::string& key) const {
        return std::find_if(known_args.begin(), known_args.end(), [&](const Argument& arg){ return arg.long_name == key; });
    }
    // 根据短命查找定义 --- 返回一个 vector 的 iterator
    std::vector<Argument>::const_iterator find_args_by_short (const std::string& key) const {
        return std::find_if(known_args.begin(), known_args.end(), [&](const Argument& arg){ return arg.short_name == key; });
    }
    // 统一存储
    void store_value(const std::string& key, const std::string& value) {
        parsed_args[key] = value;
    }
    //类型转化 字符串 -> T  流的自动解析
    template<typename T>
    T convert(const std::string& s) const {
        std::stringstream ss(s);
        T val;
        ss >> val;
        return val;
    }
};

// 特化 bool 类型的转化
template<>
bool ArgParse::convert<bool>(const std::string& s) const {
    return s == "true" || s == "1"; 
}
