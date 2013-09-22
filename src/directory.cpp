/*!
 * @brief OS毎に挙動の異なるディレクトリ操作を抽象化した関数を提供する
 * @author  koturn 0;
 * @date    2013 09/22
 * @file    directory.cpp
 * @version 0.1
 */
#include <cstdio>
#include <iostream>
#ifdef _MSC_VER
#  include <shlwapi.h>
#  include <windows.h>
#else
#  include <cstring>
#  include <unistd.h>
#  include <dirent.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#endif
#include "directory.h"


static void _getFileList(
    std::vector<std::string> &file_list,
    const std::string        &dirname);

static inline std::string trimEndOfSeparator(const std::string &path);

#ifdef _MSC_VER
/*!
 * @brief ディレクトリのファイル一覧を取得する
 * @see getFileList
 * @param [in] dirname  ディレクトリ名
 * @return  ファイル一覧
 */
static void _getFileList(
    std::vector<std::string> &file_list,
    const std::string        &dirname)
{
  WIN32_FIND_DATA wfd;
  // ハンドルを取得する
  HANDLE handle = FindFirstFileEx(
      (dirname + "*").c_str(),
      FindExInfoStandard,
      &wfd,
      FindExSearchNameMatch,
      NULL,
      0);
  if (INVALID_HANDLE_VALUE == handle) {
    std::cerr << "Error at directory:" << dirname << std::endl;
    return;
  }

  do {
    if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { // ファイルの場合
      std::string path_name = dirname + "\\" + wfd.cFileName;
      file_list.push_back(path_name);
    }
    /**
    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { // ディレクトリの場合
      // .と..は処理しない
      if (strcmp(wfd.cFileName, ".") && strcmp(wfd.cFileName, "..")) {
        _getFileList(file_list, dirname + wfd.cFileName + "\\");
      }
    } else { // ファイルの場合
      file_list.push_back(std::string(wfd.cFileName));
    }
    **/
    // 次のファイルを検索する
  } while (FindNextFile(handle, &wfd));
  // ハンドルを閉じる
  FindClose(handle);
}


#else
/*!
 * @brief ディレクトリのファイル一覧を取得する
 * @see getFileList
 * @param [in] dirname  ディレクトリ名
 * @return  ファイル一覧
 */
static void _getFileList(
    std::vector<std::string> &file_list,
    const std::string        &dirname)
{
  // ディレクトリを開く
  DIR *dir_ptr = opendir(dirname.c_str());
  if (NULL == dir_ptr) {
    std::cerr << "Error at derectory: " << dirname << std::endl;
    return;
  }

  struct dirent *ent_ptr;
  while ((ent_ptr = readdir(dir_ptr))) {
    std::string path_name = dirname + "/" + ent_ptr->d_name;
    // ファイルの情報を取得
    struct stat file_status;
    if (stat(path_name.c_str(), &file_status)) {
      printf("Failed to get stat %s \n", path_name.c_str());
      break;
    }

    if (!S_ISDIR(file_status.st_mode)) {
      file_list.push_back(path_name);
    }
    /**
    if (S_ISDIR(file_status.st_mode)) {
      // .と..は処理しない
      if (strcmp(ent_ptr->d_name, ".") && strcmp(ent_ptr->d_name, "..")) {
        _getFileList(file_list, path_name);
      }
    } else {  // ファイルの場合
      file_list.push_back(path_name);
    }
    **/
  }
  // ハンドルを閉じる
  closedir(dir_ptr);
}
#endif


/*!
 * @brief ディレクトリのファイル一覧を取得する
 *
 * 参照透過性と高速化の実現のため、再帰呼び出しと破壊的操作を施す_getFileList()<br>
 * 関数を内部で呼び出している。
 * (ただし、現在のところ再帰呼び出しの部分はコメントアウトしている)
 * また、_getFileList()関数はOSにより、内部のコードが異なる。
 * @see _getFileList
 * @param [in] dirname  ディレクトリ名
 * @return  ファイル一覧
 */
std::vector<std::string> getFileList(const std::string &dirname)
{
  std::string _dirname = trimEndOfSeparator(dirname);
  std::vector<std::string> file_list;
  _getFileList(file_list, _dirname);
  return file_list;
}




#ifdef _MSC_VER
/*!
 * @brief ファイルかディレクトリか判別する
 * @param [in] path  パス名
 * @return  指定パス名がディレクトリならばtrue、そうでなければfalse
 */
bool isDirectory(const std::string &path)
{
  return PathIsDirectory(path.c_str());
}
#else
/*!
 * @brief ファイルかディレクトリか判別する
 * @param [in] path  パス名
 * @return  指定パス名がディレクトリならばtrue、そうでなければfalse
 */
bool isDirectory(const std::string &path)
{
  std::string _path = trimEndOfSeparator(path);
  struct stat file_status;
  if (stat(_path.c_str(), &file_status)) {
    return false;
  }
  return S_ISDIR(file_status.st_mode);
}
#endif


/*!
 * @brief マス名末尾にセパレート文字があれば取り除く
 * @param [in]  パス名
 * @return  末尾にセパレート文字の無いパス名
 */
static inline std::string trimEndOfSeparator(const std::string &path)
{
  std::string _path = path;
  char ch = _path[_path.length() - 1];
  if (ch == '/') {
    _path.erase(_path.end() - 1);
  }
  return _path;
}
