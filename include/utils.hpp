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
#ifndef SCRATCH_ANALYSER_UTILS_HPP
#define SCRATCH_ANALYSER_UTILS_HPP
#pragma once
#define FMT_HEADER_ONLY
#include "bundled/fmt/core.h"
#include <vector>
#include <string>
namespace czh
{
  enum class Effect : std::size_t
  {
    bold = 1, faint, italic, underline, slow_blink, rapid_blink, color_reverse,
    fg_black = 30, fg_red, fg_green, fg_yellow, fg_blue, fg_magenta, fg_cyan, fg_white,
    bg_black = 40, bg_red, bg_green, bg_yellow, bg_blue, bg_magenta, bg_cyan, bg_white,
    bg_shadow, bg_strong_shadow
  };
  std::string effect(const std::string &str, Effect effect_)
  {
    if (str.empty()) return "";
    if(effect_ == Effect::bg_shadow)
      return fmt::format("\033[48;5;7m{}\033[49m", str);
    else if(effect_ == Effect::bg_strong_shadow)
      return fmt::format("\033[48;5;8m{}\033[49m", str);
    
    int effect = static_cast<int>(effect_);
    int end = 0;
    if (effect >= 1 && effect <= 7)
      end = 0;
    else if (effect >= 30 && effect <= 37)
      end = 39;
    else if (effect >= 40 && effect <= 47)
      end = 49;
    return fmt::format("\033[{}m{}\033[{}m", effect, str, end);
  }
  
  std::string red(const std::string &str)
  {
    return effect(str, Effect::fg_red);
  }
  std::string green(const std::string &str)
  {
    return effect(str, Effect::fg_green);
  }
  std::string yellow(const std::string &str)
  {
    return effect(str, Effect::fg_yellow);
  }
  std::string blue(const std::string &str)
  {
    return effect(str, Effect::fg_blue);
  }
  std::string magenta(const std::string &str)
  {
    return effect(str, Effect::fg_magenta);
  }
  std::string cyan(const std::string &str)
  {
    return effect(str, Effect::fg_cyan);
  }
  std::string white(const std::string &str)
  {
    return effect(str, Effect::fg_white);
  }
}

#endif //SCRATCH_ANALYSER_UTILS_HPP
