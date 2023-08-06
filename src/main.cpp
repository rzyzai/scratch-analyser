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
    std::cerr << "Usage: " << argv[0] << " Scratch_path_1 Scratch_path_2" << std::endl;
    return -1;
  }
  miniz_cpp::zip_file file1(argv[1]);
  if (!file1.has_file("project.json"))
  {
    std::cerr << "Error: " << argv[1] << " can't be loaded.";
    return -1;
  }
  miniz_cpp::zip_file file2(argv[2]);
  if (!file2.has_file("project.json"))
  {
    std::cerr << "Error: " << argv[2] << " can't be loaded.";
    return -1;
  }
  czh::Parser parser;
  auto s1 = parser.parse(nlohmann::ordered_json::parse(file1.read("project.json")));
  auto s2 = parser.parse(nlohmann::ordered_json::parse(file2.read("project.json")));
  std::cout << argv[1] << ": \n" << s1.info() << std::endl;
  std::cout << argv[2] << ": \n" << s2.info() << std::endl;
  std::cout << s1.assert_equal(s2) << std::endl;
//  auto j = nlohmann::ordered_json::parse("{\"NUM1\":[3,[12,\"过渡J2-const\",\"!aoOiWP?f/%f$S{^TI~j\"],[4,\"1\"]],\"NUM2\":[1,[4,\"20\"]]}");
//  std::vector<std::string> n;
//  czh::get_name(j, n);
//  czh::get_ids(j, n);
  return 0;
}