/*
 * stringtools.cpp
 *
 *  Created on: Sep 25, 2016
 *      Author: george
 */

#include "stringtools.hpp"

string& tools::searchReplace(string& str, const string& search,
    const string& replace) throw () {
  size_t pos = 0;
  while ((pos = str.find(search, pos)) != string::npos) {
    str.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return str;
}

static string& ltrim(string& str) throw () {
  str.erase(str.begin(),
      std::find_if(str.begin(), str.end(),
          std::not1(std::ptr_fun<int, int>(std::isspace))));
  return str;
}

static string& rtrim(string& str) throw () {
  str.erase(
      std::find_if(str.rbegin(), str.rend(),
          std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
  return str;
}

string tools::indent(int level) throw () {
  string tabs;
  for (int i = 0; i < level; i++) {
    tabs += "\t";
  }
  return tabs;
}

string& tools::trim(string& str) throw () {
  return ltrim(rtrim(str));
}

string& tools::toLowerCase(string& str) throw () {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

string& tools::toUpperCase(string& str) throw () {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}

vector<string> tools::split(const string& str, char delim) throw () {
  vector < string > elems;
  std::stringstream ss(str);
  string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

bool isCharCuttingPoint(char chr) throw () {
  const char cutting_points[] = { '\n', ' ', '-', ':', '/', '\\' };
  const int num_cutting_points = sizeof(cutting_points) / sizeof(char);
  for (int index = 0; index < num_cutting_points; ++index) {
    if (chr == cutting_points[index])
      return true;
  }
  return false;
}

void tools::breakLongLines(string& str, size_t maximum_length,
    size_t indent_length) throw () {
  string old_str(str);
  str.clear();
  string buf;
  size_t offset = 0;
  size_t next_cut_pos = 0;
  size_t current_length = 0;
  bool length_limit_hit = false;
  while (offset < old_str.length()) {
    // Reset
    buf.clear();
    if (length_limit_hit) {
      // Insert indents
      buf.insert(0, indent_length, ' ');
      buf.insert(0, 1, '\n');
      current_length = indent_length;
      length_limit_hit = false;
    } else {
      current_length = 0;
    }
    next_cut_pos = offset;

    // Copy words until either line break, length limit or end of string is
    // hit
    for (size_t i = offset;; ++i) {
      ++current_length;
      bool is_at_cutting_point = isCharCuttingPoint(old_str[i])
          || i >= old_str.length();
      if (is_at_cutting_point) {
        // Append previous words to buffer
        buf += old_str.substr(next_cut_pos, i - next_cut_pos + 1);
        next_cut_pos = i + 1;
        if (old_str[i] == '\n' || i >= old_str.length()) {
          break;
        }
      }

      if (maximum_length && current_length >= maximum_length) {
        // Remove trailing space, if present
        if (buf[buf.length() - 1] == ' ') {
          buf.erase(buf.length() - 1);
        }
        length_limit_hit = true;
        break;
      }
    }

    // If no cuts have been done, do a forcive cut
    if (next_cut_pos == offset) {
      buf += old_str.substr(offset, maximum_length);
      offset += maximum_length;
    } else {
      offset = next_cut_pos;
    }

    str += buf;
  }
}


