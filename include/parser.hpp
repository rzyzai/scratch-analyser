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
            sc.blocks.emplace_back(Block{
                .pos = pos++,
                .id = it.key(),
                .opcode = opcode,
                .input = b["inputs"],
                .fields = b["fields"],
                .shadow = shadow,
                .disabled = disabled,
                .toplevel = toplevel,
                .mutation = mutation
            });
            if (b["next"].is_string())
              sc.blocks.back().contained_id.emplace_back(b["next"].get<std::string>());
            get_ids(b["inputs"], sc.blocks.back().contained_id);
          }
        }
      }
      std::sort(sc.blocks.begin(), sc.blocks.end(), [](const Block &x, const Block &y)
      {
        if (x.opcode != y.opcode)
          return x.opcode < y.opcode;
        std::vector<std::string> xn;
        std::vector<std::string> yn;
        // input name
        get_name(x.input, xn);
        get_name(y.input, yn);
        if (xn != yn)
          return xn < yn;
        xn.clear();
        yn.clear();
        // fields name
        get_name(x.fields, xn);
        get_name(y.fields, yn);
        if (xn != yn)
          return xn < yn;
        xn.clear();
        yn.clear();
        // input id
        get_ids(x.input, xn);
        get_ids(y.input, yn);
        if (xn.size() != yn.size())
          return xn.size() < yn.size();
        xn.clear();
        yn.clear();
        // fields id
        get_ids(x.fields, xn);
        get_ids(y.fields, yn);
        if (xn.size() != yn.size())
          return xn.size() < yn.size();
        xn.clear();
        yn.clear();
        // mutation
        std::string xm;
        std::string ym;
        if (x.mutation.contains("proccode"))
          xm = x.mutation["proccode"].get<std::string>();
        if (y.mutation.contains("proccode"))
          ym = y.mutation["proccode"].get<std::string>();
        if (xm != ym)
          return xm < ym;
        return x.pos < y.pos;
      });
      return sc;
    }
  };
}
#endif //SCRATCH_ANALYSER_PARSER_HPP
