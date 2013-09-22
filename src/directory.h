/*!
 * @brief OS毎に挙動の異なるディレクトリ操作を抽象化した関数を提供する
 * @author  koturn 0;
 * @date    2013 09/22
 * @file    directory.h
 * @version 0.1
 */
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <vector>


std::vector<std::string> getFileList(const std::string &dirname);
bool isDirectory(const std::string &path);


#endif
