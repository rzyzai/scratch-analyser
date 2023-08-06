# Scratch Analyser

## 简介

适用于DobotBlock和DobotLab的Scratch Json分析库

## API

```c++
  czh::Parser parser;
// 导入文件
std::ifstream f1(argv[1]);
std::ifstream f2(argv[2]);
auto s1 = parser.parse(nlohmann::ordered_json::parse(f1));
auto s2 = parser.parse(nlohmann::ordered_json::parse(f2));
// 输出信息
std::cout << argv[1] << ": \n" << s1.info() << std::endl;
std::cout << argv[2] << ": \n" << s2.info() << std::endl;
// 比对文件
std::cout << s1.assert_equal(s2) << std::endl;
  ```

## 注意

该库只对DobotBlock和DobotLab生成的sb3文件进行过测试

## 依赖

- [nlohmann/json](https://github.com/nlohmann/json)
- [tfussell/miniz-cpp](https://github.com/tfussell/miniz-cpp)