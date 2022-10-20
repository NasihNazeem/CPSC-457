/// =========================================================================
/// Written by pfederl@ucalgary.ca in 2020, for CPSC457.
/// =========================================================================
/// You need to edit this file.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the analyzeDir() function as
/// defined in "analyzeDir.h".

#include "analyzeDir.h"

#include <cassert>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <string>
#include <algorithm>
#include <set>

long currentFileSize = 0;
long largestFileSize = 0;
long totalFileSize = 0;
long numberOfFiles = 0;
long numberOfDirs = 0;
std::string longestFilePath = "";
Results res;
constexpr int MAX_WORD_SIZE = 1024;
std::vector<std::string> vacantDirectories;


/// check if path refers to a directory
static bool is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISDIR(buff.st_mode);
}

/// check if path refers to a file
static bool is_file(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISREG(buff.st_mode);
}

/// check if string ends with another string
static bool ends_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

// ======================================================================
// You need to re-implement this function !!!!
// ======================================================================
//
// analyzeDir(n) computes stats about current directory
//   n = how many words and images to report in restuls
//
// The code below is incomplate/incorrect. Feel free to re-use any of it
// in your implementation, or delete all of it.

size_t getFileSize(const std::string& filename){

  struct stat st;
  if(stat(filename.c_str(), &st) != 0){
    return 0;
  }
  //printf("    - %s file size: %ld bytes\n", filename.c_str(), long(st.st_size));
  if(st.st_size == 0){

  }

  return st.st_size;
}

// Found in provided histogram GitLab repository:
// https://gitlab.com/-/ide/project/cpsc457/public/word-histogram/edit/master/-/main.cpp
std::string next_word(std::string filepath)
{
  std::string result;

  FILE *fp;
  //printf("FileName: %s\n", filepath.c_str());
  fp = fopen(filepath.c_str(), "r");
  if(fp){
    while(!feof(fp)) {

      int c = fgetc(fp);
      // if(c == EOF) 
      //printf("\nc is %s \n", std::to_string(c).c_str());

      // if(feof(fp)) break;
      c = tolower(c);
      if(! isalpha(c)) {
        if(result.size() == 0)
          continue;
        else
          break;
      }
      else {
        if(result.size() >= MAX_WORD_SIZE) {
          printf("input exceeded %d word size, aborting...\n", MAX_WORD_SIZE);
          exit(-1);
        } 
        result.push_back(c);
      }
    }

  }
  printf("result is: %s\n", result.c_str());

  fclose(fp);

  return result;
}


std::vector<std::pair<std::string, int>> findWords(std::string &filepath, int numberOfWords){

  std::unordered_map<std::string,int> hist;

  while(1) {
    // printf("\nI'm stuck in the while loop\n");
    auto w = next_word(filepath);
    if(w.size() == 0) break;
    hist[w] ++;
  }


  if(true) { // disabled
    printf("All words with counts:\n");
    for(auto & h : hist) {
      printf("%7d: %s\n", h.second, h.first.c_str());
    }
  }

  std::vector<std::pair<std::string,int>> arr;
  for(auto &h : hist)
    arr.emplace_back(h.first,h.second);

  if(arr.size() > size_t(numberOfWords)) {
    std::partial_sort(arr.begin(), arr.begin() + numberOfWords, arr.end());
    arr.resize(numberOfWords);
  } else{
    std::sort(arr.begin(), arr.end());
  }

  printf("%d most common words:\n", numberOfWords);
  for(auto & a : arr)
    printf("  - %s x %d\n", a.first.c_str(), a.second);

  return arr;
}


void checkVacantDirs(std::string strPath){
  int fileCount = 0;
  auto *dir = opendir(strPath.c_str());
  assert(dir != nullptr);
  for (auto de = readdir(dir); de != nullptr; de = readdir(dir)) {
    std::string name = de->d_name;
    if (name == "." or name == "..") continue;
    std::string path = strPath + "/" + name;

    if(is_file(path)){
      fileCount++;
    }
  }

  if(fileCount == 0){
    vacantDirectories.push_back(strPath);
  }
}

long largeFileSize(std::string strPath, int n){


  auto *dir = opendir(strPath.c_str());
  assert(dir != nullptr);
  for (auto de = readdir(dir); de != nullptr; de = readdir(dir)) {
    std::string name = de->d_name;
    if (name == "." or name == "..") continue;
    std::string path = strPath + "/" + name;

    if(is_file(path)){
      numberOfFiles++;
      currentFileSize = getFileSize(path);
      totalFileSize += currentFileSize;

      if (ends_with(name, ".txt")){
        //printf("    - %s is a .txt file\n", name.c_str());
        //res.most_common_words = findWords(path, n);
      }
      else
        printf("    - is not a .txt file\n");


      if(currentFileSize >= largestFileSize){
        largestFileSize = currentFileSize;
        longestFilePath = path;
      }


      continue;
    }


    if(is_dir(path)){
      numberOfDirs++;
      checkVacantDirs(path);
      currentFileSize = largeFileSize(path, n);
    }



  }

  //printf("largestFileSize is %ld\n", largestFileSize);

  return largestFileSize;
}


Results analyzeDir(int n)
{
  // The code below does a subset of the functionality you need to implement.
  // It also contains several debugging printf() statements. When you submit
  // your code for grading, please remove any dubugging pritf() statements.
  std::string dir_name = ".";

  res.largest_file_size = largeFileSize(dir_name,n);
  res.largest_file_path = longestFilePath;
  res.n_files = numberOfFiles;
  res.n_dirs = numberOfDirs;
  res.all_files_size = totalFileSize;
  res.vacant_dirs = vacantDirectories;


  // auto *dir = opendir(dir_name.c_str());
  // assert(dir != nullptr);
  // for (auto de = readdir(dir); de != nullptr; de = readdir(dir)) {
  //   std::string name = de->d_name;
  //   // skip . and .. entries
  //   if (name == "." or name == "..") continue;
  //   std::string path = dir_name + "/" + name;


  //   // printf(" - \"%s\"\n", path.c_str());

  //   // check if this is directory
  //   // if (is_dir(path)) {
  //   //   //printf("    - is a directory\n");
  //   //   continue;
  //   // }

  //   // make sure this is a file
  //   // if( not is_file(name)) {
  //   //   //printf("    - not a file and not a directory!!!\n");
  //   //   continue;
  //   // }

  //   // report size of the file
  //   // {
  //   //   struct stat buff;
  //   //   if (0 != stat(name.c_str(), &buff))
  //   //     printf("    - could not determine size of file\n");
  //   //   else
  //   //     printf("    - file size: %ld bytes\n", long(buff.st_size));

  //   // }
  //   // check if filename ends with .txt


  //   // let's see if this is an image and whether we can manage to get image info
  //   // by calling an external utility 'identify'
  //   std::string cmd = "identify -format '%wx%h' " + name + " 2> /dev/null";
  //   std::string img_size;
  //   auto fp = popen(cmd.c_str(), "r");
  //   assert(fp);
  //   char buff[PATH_MAX];
  //   if( fgets(buff, PATH_MAX, fp) != NULL) {
  //     img_size = buff;
  //   }
  //   int status = pclose(fp);
  //   if( status != 0 or img_size[0] == '0')
  //     img_size = "";
  //   if( img_size.empty())
  //     printf("    - not an image\n");
  //   else
  //     printf("    - image %s\n", img_size.c_str());
  // }
  // closedir(dir);

  // The results below are all hard-coded, to show you all the fields
  // you need to calculate. You should delete all code below and
  // replace it with your own code.
  // Results //  = "some_dir/some_file.txt";
  //  = 123;
  //  = 321;
  //  = 333;
  //  = 1000000;

  // .push_back({ "hello", 3 });
  // .push_back({ "world", 1 });

  // ImageInfo info1 { "img1", 640, 480 };
  // .push_back(info1);
  // .push_back({ "img2.png", 200, 300 });

  // .push_back("path1/subdir1");
  // .push_back("test2/xyz");
  return res;
}
