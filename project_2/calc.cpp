#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <math.h>
#include <cctype>

// 1. 基本定义

enum TokenType {
    NUMBER,     // 1 0
    OPERATOR,   // % * - + /
    FUNCTION,   // sin cos tan sqrt
    LPAREN,     // (
    RPAREN,     // )
    CONSTANT    // record pi / e
};

struct Token {
    TokenType type;
    std::string str_val;
    double num_val;
    bool is_unary;          // 专门记录是否为一元运算符
};

// 2. 词义分析器
// function:把字符串分割成token列表
class Lexer {
public:
    std::vector<Token> tokenize(const std::string& expr){
        std::vector<Token> tokens;
        for(size_t i = 0; i < expr.length(); i ++){
            // 逐一处理
            char c = expr[i];
            if(std::isspace(c))  continue;
            // 处理数字
            if(std::isdigit(c) || c == '.'){
                std::string num_str;
                while(i < expr.length() && (std::isdigit(expr[i]) || expr[i] == '.')){
                    num_str += expr[i++];
                }
                i --;
                tokens.push_back({NUMBER, num_str, std::stod(num_str), false});
            }
            // 处理字母（函数名或者常量——忽略大小写）
            else if(std::isalpha(c)){
                std::string name;
                while(i < expr.length() && isalpha(expr[i])){
                    name += expr[i ++];
                }
                i --;
                // transform(输入起点, 输入终点, 输出起点, 转换操作);
                std::string lower_name = name;
                std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                if(lower_name == "pi" || lower_name == "e"){
                    double num_val = (lower_name == "pi") ? 3.141592653589793 : 2.718281828459;
                    tokens.push_back({CONSTANT, lower_name, num_val, false});
                }else{
                    tokens.push_back({FUNCTION, lower_name, 0, false});
                }
            }
            // 处理括号
            else if(c == '('){
                tokens.push_back({LPAREN, "(", 0, false}); 
            }
            else if(c == ')'){
                tokens.push_back({RPAREN, ")", 0, false});
            }
            // 处理运算符
            else if(std::string("/*-+%^").find(c) != std::string::npos){
                bool is_unary = false;
                // 判断是运算符的减号（它必须出现在数字、右括号或常量之后）还是负号（出现在开头，或者左括号后，或者运算符后）
                if(c == '-'){
                    if(tokens.empty() || tokens.back().type == LPAREN || tokens.back().type == OPERATOR){
                        is_unary = true;
                    }
                }   
                std::string op_str(1, c);
                tokens.push_back({OPERATOR, op_str, 0, is_unary});
            }else{
                throw std::runtime_error(std::string("Unkown character: ") + c);
            }
        }
        return tokens;
    }
};

// 3. 核心计算引擎
class ScientificCalculator{
// 主要函数
private:
    // 运算符优先级
    int get_precedence(const std::string& op, bool is_unary) {
        // 负号优先级最高
        if(is_unary)    return 5;
        if(op == "^")   return 4;
        if(std::string("*%/").find(op) != std::string::npos)    return 3;
        if(op == "+" || op == "-")  return 2;
        // 0 是给括号的
        return 0;
    }
    // 结合性
    bool is_right_associative(const std::string& op)    { return op == "^"; }
    // 执行单步运算(一元 | 二元)
    double apply_op(const std::string& op, bool is_unary, std::stack<double>& values){
        if(is_unary){
            double val = values.top();  values.pop();
            if(op == "-")   return -val;
            if(op == "+")   return val;
        }
        else{
            double val2 = values.top(); values.pop();
            double val1 = values.top(); values.pop();
            if(op == "+")   return val1 + val2;
            else if(op == "-")   return val1 - val2;
            else if(op == "*")   return val1 * val2;
            else if(op == "/") {
                if(val2 == 0)   throw std::runtime_error("Division by zero");
                return val1 / val2;
            }
            else if(op == "^")   return std::pow(val1, val2);
            else if(op == "%")   return std::fmod(val1, val2);
        }
        return 0;
    }
    // 执行函数
    double apply_func(const std::string& func, double val) {
        double res = 0;
        if(func == "sin")   res = std::sin(val);
        else if(func == "cos")   res = std::cos(val);
        else if(func == "tan")   res = std::tan(val);
        else if(func == "asin")  res = std::asin(val);
        else if(func == "acos")  res = std::acos(val);
        else if(func == "atan")  res = std::atan(val);
        else if(func == "sqrt") {
            if(val < 0) throw std::runtime_error("sqrt of negative number");
            res = std::sqrt(val);
        }
        else if(func == "log")   res = std::log10(val);
        else if(func == "ln")    res = std::log(val);
        else if(func == "abs")   res = std::abs(val);
        else throw std::runtime_error("Unknow function " + func);
        return result_clean(res);
    }
// 辅助函数
public:
    // 修正浮点误差
    // 1e-14 是一个经验阈值，足够覆盖 sin(pi) 的误差
    double result_clean(double val) {
        if(std::abs(val) <= 1e-14)   return 0.0;
        return val;
    }
    // 计算
    double evaluate(const std::vector<Token>& tokens){
        std::stack<double> values;
        std::stack<Token> ops;  // 存储运算符和函数
        for(auto& token : tokens){
            switch (token.type){
                case NUMBER:

                case CONSTANT:
                    values.push(token.num_val);
                    break;

                case FUNCTION:

                case LPAREN:
                    ops.push(token);
                    break;

                case RPAREN:
                    // 遇到右括号，弹出知道左括号
                    while(!ops.empty() && ops.top().type != LPAREN) process_operator(values, ops);
                    if(ops.empty()) throw std::runtime_error("Mismatched parenthless");
                    // pop LPAREN
                    ops.pop();  

                    // 如果括号前是函数，立马计算函数
                    if(!ops.empty() && ops.top().type == FUNCTION){
                        double value = values.top();    values.pop();
                        Token op = ops.top();           ops.pop();
                        values.push(apply_func(op.str_val, value));
                    }
                    break;
                
                case OPERATOR:
                    while(!ops.empty() && ops.top().type != LPAREN){
                        Token top = ops.top();
                        // 比较优先级，栈顶是函数，优先级最高，先算，其余是经典 Shunting-yard 逻辑
                        if(top.type == FUNCTION){
                            process_operator(values, ops);
                            continue;
                        }
                        // 当前
                        int curr_prec = get_precedence(token.str_val, token.is_unary);
                        // 栈顶
                        int top_prec = get_precedence(top.str_val, top.is_unary);
                        // 经典 Shunting-yard 逻辑
                        // 优先级压制
                        // 如果操作符是右结合的（如幂运算 ^），即便优先级相同，后来的也要先算。
                        if(curr_prec < top_prec || (curr_prec == top_prec && !is_right_associative(token.str_val))){
                            process_operator(values, ops);
                        }else{
                            break;
                        }
                    }
                    ops.push(token);
                    break;
            }
        }
        while(!ops.empty()){
            if(ops.top().type == LPAREN)    throw std::runtime_error("Mismatched parenthless");
            process_operator(values, ops);
        }
        if(values.size() != 1)  throw std::runtime_error("Invalid expression");
        return values.top();
    }
private:
    // 从栈中取出操作符进行运算
    void process_operator(std::stack<double>& val_stack, std::stack<Token>& op_stack){
        Token op = op_stack.top();  op_stack.pop();
        if(op.type == FUNCTION){
            double val = val_stack.top();   val_stack.pop();
            val_stack.push(apply_func(op.str_val, val));
        }
        else if(op.type == OPERATOR){
            val_stack.push(apply_op(op.str_val, op.is_unary, val_stack));
        }
    }
};

int main(){
    Lexer lexer;
    ScientificCalculator calc;
    std::string input;

    std::cout << "========================================" << std::endl;
    std::cout << "   Casio-Style Scientific Calculator    " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Supported: + - * / ^ % ( )" << std::endl;
    std::cout << "Functions: sin, cos, tan, log, ln, sqrt, abs" << std::endl;
    std::cout << "Constants: pi, e" << std::endl;
    std::cout << "Example: sin(pi/2) * -5 + 2^3" << std::endl;
    std::cout << "Type 'exit' to quit.\n" << std::endl;

    while(true){
        std::cout << "Calc > " ;
        std::getline(std::cin, input);
        if(input == "exit") break;
        if(input.empty())   continue;
        try{
            // 1. 词义分析
            std::vector<Token> tokens = lexer.tokenize(input);
            // 2. 计算
            double ans = calc.evaluate(tokens);
            // 3. 输出
            std::cout << "= " << ans << std::endl;
        }catch (const std::exception& e){
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;   
}
// Shunting yard 函数
// 将我们习惯的“中缀表达式”（Infix Notation）转换为计算机更容易处理的“后缀表达式”（Postfix Notation，又称逆波兰表示法 RPN）。