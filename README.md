# Scratch Analyser

## 简介

适用于DobotBlock和DobotLab的Scratch Json分析库

## API

```c++
czh::Parser parser;
// 导入文件
std::ifstream file(argv[1]);
auto scratch = parser.parse(nlohmann::ordered_json::parse(file));
// 输出信息
auto info = scratch.info();
// 比对文件
auto result = scratch.compare(scratch2);
  ```

### Scratch::compare(scratch)

- 比对两个Scratch的变量、列表和程序块

#### CompareResult

- type: 比对结果
- message: 比对信息
- info: 比对详情

#### CompareResultType

- Matched 匹配
- LogicalMismatched 程序块逻辑不匹配
- ValueMismatched 程序块的输入值不匹配

## 注意

该库只对DobotBlock和DobotLab生成的sb3文件进行过测试

## 依赖

- [nlohmann/json](https://github.com/nlohmann/json)
- [tfussell/miniz-cpp](https://github.com/tfussell/miniz-cpp)
- [fmtlib](https://github.com/fmtlib/fmt)
