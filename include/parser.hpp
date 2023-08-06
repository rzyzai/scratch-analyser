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
#ifndef SCRATCH_ANALYSER_PARSER_HPP
#define SCRATCH_ANALYSER_PARSER_HPP
#pragma once
#include "scratch.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

namespace czh
{
  class Parser
  {
  public:
    Scratch parse(const nlohmann::ordered_json &project)
    {
      Scratch sc;
      for (auto &r: project["targets"])
      {
        if (r["isStage"].get<bool>())
        {
          for (auto &v: r["variables"])
          {
            if (v[1].is_string())
              sc.vars[v[0]] = v[1].get<std::string>();
            else
              sc.vars[v[0]] = to_string(v[1]);
          }
          for (auto &v: r["lists"])
          {
            std::vector<std::string> list;
            for (auto &lv: v[1])
            {
              if (lv.is_string())
                list.emplace_back(lv.get<std::string>());
              else
                list.emplace_back(to_string(lv));
            }
            sc.lists[v[0]] = list;
          }
        }
        else if (r.contains("isDevice") && r["isDevice"].get<bool>())
        {
          size_t pos = 0;
          for (auto it = r["blocks"].begin(); it != r["blocks"].end(); ++it)
          {
            auto &b = it.value();
            std::string opcode = b["opcode"].get<std::string>();
            bool shadow = b["shadow"].get<bool>();
            bool disabled = b["disabled"].get<bool>();
            bool toplevel = b["topLevel"].get<bool>();
            nlohmann::ordered_json mutation;
            if (b.contains("mutation"))
              mutation = b["mutation"];
            int x,y = -1;
            if(b.contains("x")) x = b["x"].get<int>();
            if(b.contains("y")) x = b["y"].get<int>();
            sc.blocks.emplace_back(Block{
                .pos = pos++,
                .id = it.key(),
                .parent_id = b["parent"].is_null() ? "" : b["parent"].get<std::string>(),
                .next_id = b["next"].is_null() ? "" : b["next"].get<std::string>(),
                .opcode = opcode,
                .inputs = b["inputs"],
                .fields = b["fields"],
                .shadow = shadow,
                .disabled = disabled,
                .toplevel = toplevel,
                .mutation = mutation,
                .x = x,
                .y = y
            });
          }
        }
      }
      return sc;
    }
  };
}
#endif //SCRATCH_ANALYSER_PARSER_HPP
