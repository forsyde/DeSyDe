/*
 * systools.cpp
 *
 *  Created on: Sep 25, 2016
 *      Author: george
 */


#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <ctime>
#include <stdexcept>

#include "systools.hpp"
#include "stringtools.hpp"

using namespace std;
namespace fs = boost::filesystem;


vector<string> tools::getFileNames(string inpath, string extension) throw (IOException) {

  vector<string> graphFileNames; //TODO: illegal once we go multithreading
  fs::path full_path(fs::initial_path<fs::path>());
  full_path = fs::system_complete(fs::path(inpath));

  if (!fs::exists(full_path))
    THROW_EXCEPTION(IOException, inpath, string("path does not exist"));

  if (fs::is_directory(full_path)) {
    fs::directory_iterator end_it;
    for (fs::directory_iterator dir_it(full_path);dir_it != end_it;++dir_it) {
      try {
        if (fs::is_regular_file(dir_it->path())) {
          if (extension.size()) {
            if (!dir_it->path().extension().compare(extension)) {
              string graphFileName = dir_it->path().native();
              graphFileNames.push_back(graphFileName);
            }
          } else {
            string graphFileName = dir_it->path().native();
            graphFileNames.push_back(graphFileName);
          }
        }
      }
      catch (const std::exception & ex) {
        THROW_EXCEPTION(IOException,  dir_it->path().filename().string(), string(ex.what()));
      }
    }
  }
  else {
    if (!full_path.extension().compare(extension)) {
      graphFileNames.push_back(inpath);
    }
  }

  sort(graphFileNames.begin(), graphFileNames.end());
  return graphFileNames;
}


bool tools::isAccessible(const string &inpath) throw (IOException){
  fs::path dummy(inpath + "/dummy");
  std::ofstream ofile(dummy.c_str(), std::ios::out | std::ios::trunc);
  if (!ofile) return false;
  ofile.close();
  if( remove(dummy.c_str()) )
    THROW_EXCEPTION(IOException, inpath, string("problem removing dummy file"));
  return true;
}

bool tools::isValidFilePath(const string &inpath) throw (IOException){
  fs::path full_path(fs::initial_path<fs::path>());
  full_path = fs::system_complete(fs::path(inpath));
  if (fs::is_directory(full_path)) return false;
  if (!tools::isAccessible(full_path.parent_path().string())) return false;
  return true;
}
/**
 * Converts a month name (e.g. "Jan") to its corresponding number (e.g. "01").
 * If the name is not recognized, "??" is returned.
 *
 * @param name
 *        Month name in Mmm format.
 * @returns Corresponding month number.
 */
static string monthName2Number(const string& name) throw () {
  string names[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug",
      "sep", "oct", "nov", "dec" };

  string lowercase_name(name);
  tools::toLowerCase(lowercase_name);
  int month_number;
  for (month_number = 0; month_number < 12; ++month_number) {
    if (lowercase_name.compare(names[month_number]) == 0) {
      ++month_number;
      string str;
      if (month_number < 10) {
        str += "0";
      }
      str += tools::toString(month_number);
      return str;
    }
  }

  // Name has not been recognized
  return "??";
}


string tools::getCurrentTimestamp() throw () {
  time_t raw_time;
  tm* timeinfo;

  // Get date and time data
  time(&raw_time);
  timeinfo = localtime(&raw_time);
  string raw_date(asctime(timeinfo));

  // Form timestamp
  string timestamp;
  timestamp += raw_date.substr(20, 4); // Year
  timestamp += "-";
  timestamp += monthName2Number(raw_date.substr(4, 3)); // Month
  timestamp += "-";
  string day = raw_date.substr(8, 2);
  if (day[0] == ' ')
    day[0] = '0';
  timestamp += day;
  timestamp += " ";
  timestamp += raw_date.substr(11, 8); // Time

  return timestamp;
}
