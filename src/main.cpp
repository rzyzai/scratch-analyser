// MIT License
//
// Copyright (c) 2023 caozhanhao
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "parser.hpp"
#include "miniz-cpp/zip_file.hpp"
#include <iostream>

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    fmt::println("Usage: {} sb3_path_1 sb3_path_2", argv[0]);
    return -1;
  }
  miniz_cpp::zip_file file1(argv[1]);
  if (!file1.has_file("project.json"))
  {
    fmt::println("Error: {} can't be loaded.", argv[1]);
    return -1;
  }
  miniz_cpp::zip_file file2(argv[2]);
  if (!file2.has_file("project.json"))
  {
    fmt::println("Error: {} can't be loaded.", argv[2]);
    return -1;
  }
  czh::Parser parser;
  auto s1 = parser.parse(nlohmann::ordered_json::parse(file1.read("project.json")));
  auto s2 = parser.parse(nlohmann::ordered_json::parse(file2.read("project.json")));
  fmt::println("{}:\n {}", argv[1], s1.info());
  fmt::println("{}:\n {}", argv[2], s2.info());
  auto cmp_result =  s1.compare(s2);
  bool passed = true;
  for(auto& r : cmp_result)
  {
    if(r.type == czh::CompareResultType::LogicalMismatch)
    {
      passed = false;
      fmt::println("{}: {}\n{}", czh::red("Logical Mismatch"), r.message, r.info);
    }
  }
  if(passed)
    fmt::println(czh::green("Passed."));
  return 0;
}