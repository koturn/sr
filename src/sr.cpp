/*!
 * @brief クラウド型音声合成を利用した音声認識プログラム
 * @author  Koji Ueta
 * @date    2013 09/22
 * @file    sr.cpp
 * @version 0.2
 */
// C++の標準ライブラリ
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <sstream>
// インクルードパスに追加する必要のあるライブラリ
#include <clx/http.h>
#include <clx/uri.h>
#include <clx/base64.h>
#include <picojson/picojson.h>
// 自作ユーティリティ
#include "directory.h"

// attributeの指定が使えない処理系では、attributeを消す
#ifndef __GNUC__
#  define __attribute__(attr)
#endif


static int parseArguments(
    int          argc,
    char        *argv[],
    std::string &lang);

__attribute__((noreturn))
static inline void showUsage(const char *progname, int exit_status);

static inline bool checkSuffix(
    const std::string &filename,
    const std::string &suffix);

static inline std::string removeSuffix(const std::string &filename);

static void speachToText(
          clx::http   &session,
    const std::string &lang,
    const std::string &wav_filename,
    const std::string &filename);

static inline std::string wavToBase64(const std::string &wav_filename);

__attribute__((const))
static inline std::string makeJsonString(
    const std::string &lang,
    const std::string &base64_wav);

__attribute__((const))
static inline std::string quoteString(const std::string &str);

static std::string jsonToResult(const std::string &ret_json);


//! 接続先ポート番号
static const int HTTP_PORT = 80;
//! サーバのホスト部分
static const std::string URL_HOST = "rospeex.ucri.jgn-x.jp";
//! サーバのパス部分
static const std::string URL_PATH = "/nauth_json/jsServices/VoiceTraSR";
//! デフォルトの入力言語
static const std::string DEFAULT_LANG = "ja";
//! 送信するJSON文字列の先頭部分
static const std::string JSON_PART1 =
"{"
  "\"method\": \"recognize\","
  "\"params\": [";
//! 送信するJSON文字列の中間部分
static const std::string JSON_PART2 =
    ","
    "{"
      "\"audio\": ";
//! 送信するJSON文字列の末尾
static const std::string JSON_PART3 =
      ","
      "\"audioType\": \"audio/x-wav\","
      "\"voiceType\": \"*\""
    "}"
  "]"
"}";


/*!
 * @brief プログラムのエントリポイント
 * @param [in] argc コマンドライン引数の数
 * @param [in] argv コマンドライン引数
 * @return  終了ステータス
 */
int main(int argc, char *argv[])
{
  std::string lang = DEFAULT_LANG;
  int remidx = parseArguments(argc, argv, lang);
  if (argc == remidx) {  // 引数のチェック
    std::cerr << "Invalid argument" << std::endl;
    showUsage(argv[0], EXIT_FAILURE);
  }

  try {
    std::vector<std::string> arg_list;
    for (int i = remidx; i < argc; i++) {
      arg_list.push_back(argv[i]);
    }
    clx::http session(clx::uri::encode(URL_HOST), HTTP_PORT);
    for (std::vector<std::string>::iterator arg = arg_list.begin(); arg != arg_list.end(); ++arg) {
      if (isDirectory(*arg)) {  // ディレクトリだった場合
        std::vector<std::string> file_list = getFileList(*arg);
        for (std::vector<std::string>::iterator fn = file_list.begin(); fn != file_list.end(); ++fn) {
          if (checkSuffix(*fn, "wav")) {
            std::cout << *fn << " ... ";
            try {
              speachToText(session, lang, *fn, *fn + ".txt");
              std::cout << "Proccessed" << std::endl;
            } catch (const char *errmsg) {
              std::cerr << errmsg << std::endl;
            }
          }
        }
      } else {  // ファイルだった場合
        std::cout << *arg << " ... ";
        try {
          speachToText(session, lang, *arg, *arg + ".txt");
          std::cout << "Proccessed" << std::endl;
        } catch (const char *errmsg) {
          std::cerr << errmsg << std::endl;
        }
      }
    }
  } catch (clx::socket_error &e) {
    std::cerr << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}


/*!
 * @brief コマンドライン引数の解析を行う
 *
 * 引数argvのうち、オプションに関係しないものは、後ろに並び替えられる。
 * その開始インデックスを返り値とする
 * @param [in]     argc      コマンドライン引数の数
 * @param [in,out] argv      コマンドライン引数
 * @param [out]    lang      音声合成の言語
 * @param [out]    filename  出力ファイル名
 * @return  残った引数のインデックス
 */
static int parseArguments(
    int          argc,
    char        *argv[],
    std::string &lang)
{
  static const struct option opts[] = {
    {"help",     no_argument,       NULL, 'h'},
    {"language", required_argument, NULL, 'l'},
    {0, 0, 0, 0}   // must be filled with zero
  };

  int ret;
  int optidx;
  while ((ret = getopt_long(argc, argv, "hl:", opts, &optidx)) != -1) {
    switch (ret) {
      case 'h':  // -h or --help
        showUsage(argv[0], EXIT_SUCCESS);
        break;
      case 'l':  // -l or --language
        lang = optarg;
        break;
    }
  }
  return optind;
}


/*!
 * @brief 使い方を表示し、プログラムを終了する
 * @attention この関数は呼び出し元に戻らないので、g++/clangではnoreturn属性を付加した
 * @param [in] progname     プログラム名
 * @param [in] exit_status  終了ステータス
 */
static inline void showUsage(const char *progname, int exit_status)
{
  std::cout << "[Usage]" << std::endl
            << "  $ " << progname << " [text] {options}" << std::endl
            << std::endl
            << "[options]" << std::endl
            << "  -h, --help" << std::endl
            << "    Show usage of this program" << std::endl
            << "  -l [audio-language], --language=[audio-language]" << std::endl
            << "    DEFAULT VALUE: ja" << std::endl
            << "    Specify language of audio" << std::endl;
  exit(exit_status);
}


/*!
 * @brief ファイル名の拡張子をを判別する
 * @param [in] filename  ファイル名
 * @param [in] suffix    拡張子(ドットは含まない)
 * @return  指定したファイル名の拡張子と指定拡張子が合致していればtrue、
 *          そうでなければfalse
 */
static inline bool checkSuffix(
    const std::string &filename,
    const std::string &suffix)
{
  int idx = filename.find_last_of(".");
  if (idx == -1) {
    return -1;
  }
  return filename.substr(idx + 1) == suffix;
}


/*!
 * @brief ファイル名の拡張子を覗いた部分を返す
 * @param [in] filename  ファイル名
 * @return  ファイル名の拡張子を覗いた部分
 */
static inline std::string removeSuffix(const std::string &filename)
{
  int idx = filename.find_last_of(".");
  return idx == -1 ? "" : filename.substr(0, idx);
}


/*!
 * @brief 音声認識を行う
 * @param [in] lang          messageの言語
 * @param [in] wav_filename  音声認識対象のwavファイル
 * @param [in] filename      出力ファイル名
 */
static void speachToText(
          clx::http   &session,
    const std::string &lang,
    const std::string &wav_filename,
    const std::string &filename)
{
  // wavファイルをbase64でエンコード
  std::string base64_wav = wavToBase64(wav_filename);
  // サーバに投げるJSON文字列の作成
  std::string json_str = makeJsonString(lang, base64_wav);

  // 例外はmain側に投げる
  session.post(clx::uri::encode(URL_PATH), json_str);  // JSONをサーバに投げる
  std::string body_str = session.body();               // 受信したデータの本体(JSON)を取得
  std::string result   = jsonToResult(body_str);       // JSONからwavに変換

  std::ofstream ofs(filename.c_str(), std::ofstream::binary);
  if (!ofs) {
    throw "Cannot open " + filename;
  }
  ofs << result;
  ofs.close();
}


/*!
 * @brief wavファイルをbase64でエンコードする
 * @param [in] wav_filename  エンコードしたいwavファイル名
 * @return  base64でエンコードされたwavバイナリ
 */
static inline std::string wavToBase64(const std::string &wav_filename)
{
  std::ifstream ifs(wav_filename.c_str(), std::ifstream::binary);
  if (!ifs) {
    throw "Cannot open " + wav_filename;
  }
  // 読み込み中エラーのための例外設定
  ifs.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);

  // ファイルサイズの取得(バイナリ全体を読み込むため)
  ifs.seekg(0, std::ifstream::end);
  std::streamoff eof_pos = ifs.tellg();
  ifs.clear();
  ifs.seekg(0, std::ifstream::beg);
  std::streamoff beg_pos = ifs.tellg();
  size_t data_size = eof_pos - beg_pos;

  try {
    char *bin_data = new char[data_size];
    ifs.read(bin_data, data_size);  // wavファイル全体を読み込む
    ifs.close();  // ファイル読み込みストリームのクローズ
    std::string base64_wav = clx::base64::encode(bin_data, data_size);
    delete bin_data;
    return base64_wav;
  } catch (std::bad_alloc) {
    throw "Failed to allocate memory";
  } catch (std::ios_base::failure) {
    throw "An error occured while read a file: " + wav_filename;
  }
  return "";
}


/*!
 * @brief サーバに投げるJSON文字列を作成する
 * @param [in] lang        wavファイルに収録されている言語
 * @param [in] base64_wav  base64でエンコードされたwavバイナリ
 * @return  サーバに投げるJSON文字列
 */
static inline std::string makeJsonString(
    const std::string &lang,
    const std::string &base64_wav)
{
  return JSON_PART1 + quoteString(lang)
    + JSON_PART2 + quoteString(base64_wav)
    + JSON_PART3;
}


/*!
 * @brief 文字列をダブルクオートで囲う
 * @param [in] str  クオートしたい文字列
 * @return  クオートされた文字列
 */
static inline std::string quoteString(const std::string &str)
{
  return "\"" + str + "\"";
}


/*!
 * @brief サーバから受信したJSON文字列から認識結果を取り出す
 * @param [in] ret_json  サーバから受信したJSON文字列
 * @return  認識結果
 */
static std::string jsonToResult(const std::string &ret_json)
{
  picojson::value  pico_value;
  const char      *ret_json_cstr = ret_json.c_str();
  std::string      err;
  picojson::parse(pico_value, ret_json_cstr, ret_json_cstr + strlen(ret_json_cstr), &err);

  // JSON全体のオブジェクトを取得
  picojson::object &pico_obj = pico_value.get<picojson::object>();
  /**
  if (pico_obj["error"].get<std::string>() != "null") {
    throw "Failed speach recognition";
  }
  **/
  // キー: resultに対応するサブのJSONを取得
  std::string result = pico_obj["result"].get<std::string>();
  return result;
}
