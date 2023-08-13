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
#include "utils.hpp"
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

//  void get_ids(const nlohmann::ordered_json &j, std::vector<std::string> &ret)
//  {
//    if (j.is_array() || j.is_object())
//    {
//      for (auto it = j.begin(); it != j.end(); ++it)
//      {
//        if (it->is_object() && it.key().size() == 20)
//          ret.emplace_back(it.key());
//        else
//          get_ids(it.value(), ret);
//      }
//    }
//    else if (j.is_string())
//    {
//      if (auto s = j.get<std::string>(); s.size() == 20)
//        ret.emplace_back(s);
//    }
//  }
//
  
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
  auto find_if_with_hint(const typename std::vector<T>::const_iterator &it1,
                         const typename std::vector<T>::const_iterator &it2,
                         const typename std::vector<T>::const_iterator &hint,
                         const std::function<bool(const T &)> &pred)
  {
    if (pred(*hint))
      return hint;
    auto r1 = std::find_if(hint, it2, pred);
    if (r1 != it2) return r1;
    auto r2 = std::find_if(it1, hint, pred);
    if (r2 != hint) return r2;
    return it2;
  }
  
  template<typename T>
  auto find_with_hint(const typename std::vector<T>::const_iterator &it1,
                      const typename std::vector<T>::const_iterator &it2,
                      const typename std::vector<T>::const_iterator &hint,
                      const T &v)
  {
    return find_if_with_hint<T>(it1, it2, hint, [&v](auto &&t) { return v == t; });
  }
  
  enum class CompareResultType
  {
    Matched, LogicalMismatch, ValueMisMatch
  };
  struct CompareResult
  {
    CompareResultType type;
    std::string message;
    std::string info;
  };
  
  template<typename T>
  struct BasicValue
  {
    std::string name;
    T value;
  public:
    bool operator==(const BasicValue &b) const
    {
      return name == b.name;
    }
    
    std::string info() const
    {
      return fmt::format("  name: {}\n  value: {}", name, nlohmann::ordered_json(value).dump());
    }
    
    std::vector<CompareResult> compare(const BasicValue &b) const
    {
      std::vector<CompareResult> ret;
      if (name != b.name)
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different name of variables.",
            .info = fmt::format("({}, {})", name, b.name)
        });
      }
      if (value != b.value)
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::ValueMisMatch,
            .message = "Different value of variables.",
            .info = fmt::format("({}, {}, {})", name,
                                nlohmann::ordered_json(value).dump(),
                                nlohmann::ordered_json(b.value).dump())
        });
      }
      return ret;
    }
  };
  
  using Variable = BasicValue<std::string>;
  using List = BasicValue<std::vector<std::string>>;
  
  struct Block
  {
    size_t pos;
    std::string id;
    std::string parent_id;
    std::string next_id;
    std::string opcode;
    nlohmann::ordered_json inputs;
    nlohmann::ordered_json fields;
    bool shadow;
    bool disabled;
    bool toplevel;
    nlohmann::ordered_json mutation;
    int x;
    int y;
  public:
    std::string info() const
    {
      return fmt::format("  id: {}\n  opcode: {}\n  inputs: {}\n  fields: {}\n  shadow: {}\n"
                         "  disabled: {}\n  toplevel: {}\n  mutation: {}",
                         id, opcode, inputs.dump(), fields.dump(), shadow, disabled, toplevel, mutation.dump());
    }
    
    bool operator==(const Block &b) const
    {
      return opcode == b.opcode
             && is_format_equal(inputs, b.inputs)
             && is_format_equal(fields, b.fields)
             && shadow == b.shadow
             && disabled == b.disabled
             && toplevel == b.toplevel
             && is_format_equal(mutation, b.mutation);
    }
    
    std::vector<CompareResult> compare(const Block &b) const
    {
      std::vector<CompareResult> ret;
      if (opcode != b.opcode)
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different Block opcode.",
            .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                info(), b.info())
        });
      }
      else if (inputs != b.inputs)
      {
        if (is_format_equal(inputs, b.inputs))
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::ValueMisMatch,
              .message = "Different Block inputs value",
              .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                  info(), b.info())
          });
        }
        else
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::LogicalMismatch,
              .message = "Different Block inputs format",
              .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                  info(), b.info())
          });
        }
      }
      else if (fields != b.fields)
      {
        if (is_format_equal(fields, b.fields))
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::ValueMisMatch,
              .message = "Different Block fields value",
              .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                  info(), b.info())
          });
        }
        else
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::LogicalMismatch,
              .message = "Different Block inputs format",
              .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                  info(), b.info())
          });
        }
      }
      else if (shadow != b.shadow)
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different Block shadow",
            .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                info(), b.info())
        });
      }
      else if (disabled != b.disabled)
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different Block disabled",
            .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                info(), b.info())
        });
      }
      else if (toplevel != b.toplevel)
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different Block toplevel",
            .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                info(), b.info())
        });
      }
      else if (mutation != b.mutation)
      {
        if (is_format_equal(mutation, b.mutation))
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::ValueMisMatch,
              .message = "Different Block mutation value",
              .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                  info(), b.info())
          });
        }
        else
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::LogicalMismatch,
              .message = "Different Block mutation format",
              .info = fmt::format("Block Info 1: {}\nBlock Info 2: {}",
                                  info(), b.info())
          });
        }
      }
      return ret;
    }
  };
  
  const Block *to_parent(const std::vector<Block> &v, const Block *block)
  {
    if (block->parent_id.empty()) return nullptr;
    const Block *parent = nullptr;
    for (size_t i = 0; i < v.size(); ++i)
    {
      if (v[i].id == block->parent_id)
      {
        parent = &v[i];
        break;
      }
    }
    return parent;
  }
  
  const Block *to_toplevel(const std::vector<Block> &v, const Block *block)
  {
    auto parent = to_parent(v, block);
    if (parent == nullptr)
      return block;
    return to_toplevel(v, parent);
  }
  
  // TODO  Not very strong
  // There might be bugs
  bool strong_block_equal(const std::vector<Block> &bs1, const std::vector<Block> &bs2, const Block &block1,
                          const Block &block2)
  {
    if (block1 == block2)
    {
      auto p1 = to_toplevel(bs1, &block1);
      auto p2 = to_toplevel(bs2, &block2);
      if ((p1 == nullptr && p2 != nullptr) || (p1 != nullptr && p2 == nullptr))
        return false;
      if (p1 == nullptr)
        return true;
      else
        return *p1 == *p2;
    }
    else
      return false;
  }
  
  template<typename T>
  void match_basic_value(const std::vector<T> &bs1, const std::vector<T> &bs2, const std::string &name,
                         const std::string &type,
                         std::vector<CompareResult> &ret)
  {
    for (auto it
        = bs1.cbegin(); it != bs1.cend(); ++it)
    {
      auto r = find_with_hint<T>(bs2.cbegin(), bs2.cend(), bs2.cbegin() + (it - bs1.cbegin()), *it);
      if (r == bs2.cend())
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = fmt::format("Scratch {} has a mismatched {}.", name, type),
            .info = fmt::format("{} Info:\n{}", type, it->info())
        });
      }
      else
      {
        auto cmp_result = it->compare(*r);
        ret.insert(ret.end(), std::make_move_iterator(cmp_result.begin()),
                   std::make_move_iterator(cmp_result.end()));
      }
    }
  }
  
  void match_block(const std::vector<Block> &bs1, const std::vector<Block> &bs2,
                   const std::string &name, std::vector<CompareResult> &ret)
  {
    for (auto it
        = bs1.cbegin(); it != bs1.cend(); ++it)
    {
      auto r = find_if_with_hint<Block>(
          bs2.cbegin(), bs2.cend(),
          bs2.cbegin() + (it - bs1.cbegin()),
          [&it, &bs1, &bs2](const Block &block) -> bool
          {
            return strong_block_equal(bs1, bs2, *it, block);
          });
      if (r == bs2.cend())
      {
        const Block *b = &*it;
        auto tb = to_toplevel(bs1, b);
        std::string tb_info = "Failed to get toplevel.";
        if (tb != nullptr)
          tb_info = tb->info();
        
        if (tb != nullptr && tb->opcode != "procedures_definition")
        {
          ret.emplace_back(CompareResult{
              .type = CompareResultType::LogicalMismatch,
              .message = fmt::format("Scratch {} has a mismatched Block.", name),
              .info = fmt::format("Block Info:\n{}\nTopLevelBlock Info:\n{}",
                                  it->info(), tb_info)
          });
        }
        else if (tb != nullptr)
        {
          std::string related_info = "Failed to get related block.";
          for (auto &rel: bs1)
          {
            if (rel.parent_id == tb->id && rel.opcode == "procedures_prototype")
            {
              related_info = rel.info();
              break;
            }
          }
          ret.emplace_back(CompareResult{
              .type = CompareResultType::LogicalMismatch,
              .message = fmt::format("Scratch {} has a mismatched Block.", name),
              .info = fmt::format("Block Info:\n{}\nTopLevelBlock Info:\n{}\nRelatedBlock:\n{}",
                                  it->info(), tb_info, related_info)
          });
        }
      }
      else
      {
        auto cmp_result = it->compare(*r);
        ret.insert(ret.end(), std::make_move_iterator(cmp_result.begin()),
                   std::make_move_iterator(cmp_result.end()));
      }
    }
  }
  
  class Parser;
  
  class Scratch
  {
    friend Parser;
  private:
    std::string name;
    std::vector<Variable> vars;
    std::vector<List> lists;
    std::vector<Block> blocks;
  public:
    std::string info() const
    {
      return fmt::format("Variables: {}, Lists: {}, Blocks: {}", vars.size(), lists.size(), blocks.size());
    }
    
    std::vector<CompareResult> compare(const Scratch &sc) const
    {
      std::vector<CompareResult> ret;
      // Variables
      if (vars.size() != sc.vars.size())
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different number of variables.",
            .info = fmt::format("({}, {})", vars.size(), sc.vars.size())
        });
      }
      
      match_basic_value<Variable>(vars, sc.vars, name, "Variable", ret);
      match_basic_value<Variable>(sc.vars, vars, sc.name, "Variable", ret);
      
      // Lists
      if (lists.size() != sc.lists.size())
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different number of lists.",
            .info = fmt::format("({}, {})", lists.size(), sc.lists.size())
        });
      }
      
      match_basic_value<List>(lists, sc.lists, name, "List", ret);
      match_basic_value<List>(sc.lists, lists, sc.name, "List", ret);
      // Blocks
      if (blocks.size() != sc.blocks.size())
      {
        ret.emplace_back(CompareResult{
            .type = CompareResultType::LogicalMismatch,
            .message = "Different number of blocks.",
            .info = fmt::format("({}, {})", blocks.size(), sc.blocks.size())
        });
      }
      match_block(blocks, sc.blocks, name, ret);
      match_block(sc.blocks, blocks, sc.name, ret);
      return ret;
    }
  };
}
#endif //SCRATCH_ANALYSER_SCRATCH_HPP
