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
#ifndef SCRATCH_ANALYSER_SCRATCH_HPP
#define SCRATCH_ANALYSER_SCRATCH_HPP
#pragma once
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

namespace czh
{
  
  void get_name(const nlohmann::ordered_json &j, std::vector<std::string> &ret)
  {
    if (j.is_array() || j.is_object())
    {
      for (auto it = j.begin(); it != j.end(); ++it)
      {
        if (j.is_array() && it.value().is_string() && it.value().get<std::string>().size() == 20 &&
            (it - 1) >= j.begin() && (it - 1).value().is_string())
          ret.emplace_back((it - 1).value().get<std::string>());
        else
          get_name(it.value(), ret);
      }
    }
  }
  
  void get_ids(const nlohmann::ordered_json &j, std::vector<std::string> &ret)
  {
    if (j.is_array() || j.is_object())
    {
      for (auto it = j.begin(); it != j.end(); ++it)
      {
        if (it->is_object() && it.key().size() == 20)
          ret.emplace_back(it.key());
        else
          get_ids(it.value(), ret);
      }
    }
    else if (j.is_string())
    {
      if (auto s = j.get<std::string>(); s.size() == 20)
        ret.emplace_back(s);
    }
  }
  bool is_format_equal(const nlohmann::ordered_json &j1, const nlohmann::ordered_json &j2)
  {
    if (j1.type() != j2.type())
      return false;
    if (j1.is_object() || j1.is_array())
    {
      if (j1.size() != j2.size())
        return false;
      for (auto it1 = j1.begin(), it2 = j2.begin(); it1 != j1.end(); ++it1, ++it2)
      {
        if (!is_format_equal(it1.value(), it2.value()))
          return false;
      }
    }
    std::vector<std::string> n1;
    std::vector<std::string> n2;
    get_name(j1, n1);
    get_name(j2, n2);
    return n1 == n2;
  }
  
  template<typename T>
  auto find_with_hint(const typename std::vector<T>::const_iterator& it1,
                      const typename std::vector<T>::const_iterator& it2,
                      const typename std::vector<T>::const_iterator& hint,
                      const std::function<bool(const T&)>& pred)
  {
    if(pred(*hint))
      return hint;
    auto r1 = std::find_if(hint, it2, pred);
    if(r1 != it2) return r1;
    auto r2 = std::find_if(it1, hint, pred);
    if(r2 != hint) return r2;
    return it2;
  }
  
  
  
  struct Block
  {
    size_t pos;
    std::string id;
    std::string parent_id;
    std::vector<std::string> contained_id;
    std::string opcode;
    nlohmann::ordered_json input;
    nlohmann::ordered_json fields;
    bool shadow;
    bool disabled;
    bool toplevel;
    nlohmann::ordered_json mutation;
    int x;
    int y;
  public:
    std::string to_string(size_t indent) const
    {
      std::string space(indent, ' ');
      std::string nl_space = "\n";
      nl_space.insert(nl_space.end(), indent, ' ');
      return space + "id: " + id + nl_space + "opcode: " + opcode + nl_space + "input: " + input.dump() + nl_space +
             "fields: " + fields.dump()
             + nl_space + "shadow: " + (shadow ? "True" : "False") + nl_space + "disabled: " +
             (disabled ? "True" : "False")
             + nl_space + "toplevel: " + (toplevel ? "True" : "False") + nl_space + "mutation: " + mutation.dump() +
             "\n";
    }
    
    bool operator==(const Block& b) const
    {
      return opcode == b.opcode
      && is_format_equal(input, b.input)
      && is_format_equal(fields, b.fields)
      && shadow == b.shadow
      && disabled == b.disabled
      && toplevel == b.toplevel
      && is_format_equal(mutation, b.mutation);
    }

    std::tuple<bool, std::string, size_t, size_t> assert_equal(const Block& b) const
    {
      std::string ret;
      size_t logic_mismatch, value_mismatch = 0;
      auto report_block = [&ret](const Block &block1, const Block &block2)
      {
        ret += "Block1 Info:\n" + block1.to_string(4);
        ret += "Block2 Info:\n" + block2.to_string(4);
      };
      if (opcode != b.opcode)
      {
        ret += "\u001b[31mLogic Mismatch\u001b[0m: Different Block opcode (" + opcode + ", " + b.opcode + ").\n";
        report_block(*this, b);
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else if (input != b.input)
      {
        if (is_format_equal(input, b.input))
        {
          ret += "\u001b[33mValue Mismatch\u001b[0m: Different Block input value (" + input.dump() + ", " +
                 b.input.dump() + ").\n";
          //report_block(*this, b);
          ++value_mismatch;
        }
        else
        {
          ret += "\u001b[31mLogic Mismatch\u001b[0m: Different Block input format (" + input.dump() + ", " +
                 b.input.dump() + ").\n";
          report_block(*this, b);
          ret += "\u001b[31mTerminated\u001b[0m.\n";
          ++logic_mismatch;
          //return ret;
        }
      }
      else if (fields != b.fields)
      {
        if (is_format_equal(fields, b.fields))
        {
          ret += "\u001b[33mValue Mismatch\u001b[0m: Different Block fields value (" + fields.dump() + ", " +
                 b.fields.dump() + ").\n";
          //report_block(*this, b);
          ++value_mismatch;
        }
        else
        {
          ret += "\u001b[31mLogic Mismatch\u001b[0m: Different Block fields format (" + fields.dump() + ", " +
                 b.fields.dump() + ").\n";
          report_block(*this, b);
          ret += "\u001b[31mTerminated\u001b[0m.\n";
          ++logic_mismatch;
          //return ret;
        }
      }
      else if (shadow != b.shadow)
      {
        ret += std::string("\u001b[31mLogic Mismatch\u001b[0m: Different Block shadow (")
               + (shadow ? "T" : "F") + ", " + (b.shadow ? "T" : "F") + std::string(").\n");
        report_block(*this, b);
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else if (disabled != b.disabled)
      {
        ret += std::string("\u001b[31mLogic Mismatch\u001b[0m: Different Block disabled (")
               + (disabled ? "T" : "F") + ", " + (b.disabled ? "T" : "F") + std::string(").\n");
        report_block(*this, b);
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else if (toplevel != b.toplevel)
      {
        ret += std::string("\u001b[31mLogic Mismatch\u001b[0m: Different Block toplevel (")
               + (toplevel ? "T" : "F") + ", " + (b.toplevel ? "T" : "F") + std::string(").\n");
        report_block(*this, b);
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else if (mutation != b.mutation)
      {
        if (is_format_equal(mutation, b.mutation))
        {
          ret += "\u001b[33mValue Mismatch\u001b[0m: Different Block mutation value (" + mutation.dump() + ", " +
                 b.mutation.dump() + ").\n";
          //report_block(*this, b);
          ++value_mismatch;
        }
        else
        {
          ret += "\u001b[31mLogic Mismatch\u001b[0m: Different Block mutation format (" + mutation.dump() + ", " +
                 b.mutation.dump() + ").\n";
          report_block(*this, b);
          ret += "\u001b[31mTerminated\u001b[0m.\n";
          ++logic_mismatch;
          //return ret;
        }
      }
      return {(logic_mismatch == 0), ret, logic_mismatch, value_mismatch};
    }
  };
  
  class Parser;
  class Scratch
  {
    friend Parser;
  private:
    std::map<std::string, std::string> vars;
    std::map<std::string, std::vector<std::string>> lists;
    std::vector<Block> blocks;
  public:
    std::string info() const
    {
      return "variables: " + std::to_string(vars.size())
             + "\nlists: " + std::to_string(lists.size())
             + "\nblocks: " + std::to_string(blocks.size()) + "\n";
    }
    
    std::string assert_equal(const Scratch &sc) const
    {
      std::string ret;
      size_t value_mismatch = 0;
      size_t logic_mismatch = 0;
      // Variables
      if (vars.size() != sc.vars.size())
      {
        ret += "\u001b[31mLogic Mismatch\u001b[0m: Different number of variables\n";
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else
      {
        for (auto it1 = vars.cbegin(), it2 = sc.vars.cbegin(); it1 != vars.cend(); ++it1, ++it2)
        {
          if (it1->first != it2->first)
          {
            ret += "\u001b[31mLogic Mismatch\u001b[0m: Different Name of variables (" + it1->first + ", " + it2->first + ").\n";
            ret += "\u001b[31mTerminated\u001b[0m.\n";
            ++logic_mismatch;
            //return ret;
          }
        }
        for (auto it1 = vars.cbegin(), it2 = sc.vars.cbegin(); it1 != vars.cend(); ++it1, ++it2)
        {
          if (it1->second != it2->second)
            ret +=
                "\u001b[33mValue Mismatch\u001b[0m: Different value of variables ([" + it1->first + "] " + it1->second + ", " +
                it2->second + ").\n";
          ++value_mismatch;
        }
      }
      // Lists
      if (lists.size() != sc.lists.size())
      {
        ret += "\u001b[31mLogic Mismatch\u001b[0m: Different number of lists\n";
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else
      {
        for (auto it1 = lists.cbegin(), it2 = sc.lists.cbegin(); it1 != lists.cend(); ++it1, ++it2)
        {
          if (it1->first != it2->first)
          {
            ret += "\u001b[31mLogic Mismatch\u001b[0m: Different Name of lists (" + it1->first + ", " + it2->first + ").\n";
            ret += "\u001b[31mTerminated\u001b[0m.\n";
            ++logic_mismatch;
            //return ret;
          }
        }
        for (auto it1 = lists.cbegin(), it2 = sc.lists.cbegin(); it1 != lists.cend(); ++it1, ++it2)
        {
          if (it1->second != it2->second)
            ret += "\u001b[33mValue Mismatch\u001b[0m: Different value of lists ([" + it1->first + "] "
                   + nlohmann::ordered_json{it1->second}.dump() + ", " + nlohmann::ordered_json{it2->second}.dump() +
                   ").\n";
          ++value_mismatch;
        }
      }
      // Blocks
      if (blocks.size() != sc.blocks.size())
      {
        ret += "\u001b[31mLogic Mismatch\u001b[0m: Different number of blocks\n";
        ret += "\u001b[31mTerminated\u001b[0m.\n";
        ++logic_mismatch;
        //return ret;
      }
      else
      {
        for (auto it1 = blocks.cbegin(); it1 != blocks.cend(); ++it1)
        {
          auto r = find_with_hint<Block>(
              sc.blocks.cbegin(), sc.blocks.cend(), sc.blocks.cbegin() + (it1 - blocks.cbegin()),
              [it1](const Block &b) -> bool
              {
                // return std::get<0>(it1->assert_equal(b));
                return *it1 == b;
              });
          if (r == sc.blocks.end())
          {
            ret += "\u001b[31mLogic Mismatch\u001b[0m: Mismatched Block\n";
            ret += "Block Info:\n" + it1->to_string(4);
            ret += "\u001b[31mTerminated\u001b[0m.\n";
            ++logic_mismatch;
          }
          else
          {
            auto[ok, ret_b, l, v] = it1->assert_equal(*r);
            ret += ret_b;
            value_mismatch += v;
          }
        }
      }
      if (logic_mismatch == 0)
        ret += "\u001b[32mPassed\u001b[0m: Value Mismatches: " + std::to_string(value_mismatch) + ", Logic Mismatchs: " + std::to_string(logic_mismatch) +
          "\n";
      else
        ret +=
            "\u001b[31Failed\u001b[0m: Value Mismatches: " + std::to_string(value_mismatch) + ", Logic Mismatchs: " + std::to_string(logic_mismatch) +
            "\n";
      return ret;
    }
  };
  
}
#endif //SCRATCH_ANALYSER_SCRATCH_HPP
