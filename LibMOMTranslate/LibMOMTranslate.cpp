#include <iostream>
#include <fstream>

#include "pch.h"
#include "framework.h"
#include "LibMOMTranslate.h"

#include <Shlobj.h>
#include <pathcch.h>

constexpr auto DEFAULT_TARGET_LANG = "en";
constexpr auto APP_CONFIG_FILE_NAME = (const wchar_t*) L".momtrans.config.ini" ;

bool getConfigPath(PWSTR buffer, size_t bufferSize) {
  wchar_t* appDataPath;

  if (SHGetKnownFolderPath(FOLDERID_LocalAppData,
                           KF_FLAG_DEFAULT,
                           NULL,
                           &appDataPath) == S_OK) {
    PathCchCombine(buffer, bufferSize, appDataPath, APP_CONFIG_FILE_NAME);
    CoTaskMemFree(appDataPath);
    return true;
  }
  return false;
}

bool iniConfig(CSimpleIniA &ini) {
  PWSTR path = (PWSTR)malloc(sizeof(PWSTR) * 256);
  if (!getConfigPath(path, 256)) return false;

  bool success = false;

  std::fstream configFStream;
  configFStream.open(path, std::ios::out | std::ios::app);
  configFStream.close();

  ini.SetUnicode();
  SI_Error err = ini.LoadFile(path);

  free(path);
  return err < 0;
}

bool translate(const char* text, char* buffer, size_t bufferSize) {
  char targetLang[10];

  cpr::Session session;
  cpr::Body body;
  cpr::Response response;

  bool readTargetLang = readConfigValue(TRANSLATOR_SECTION,
                                        TRANSLATOR_TARGET_LANG_KEY,
                                        DEFAULT_TARGET_LANG,
                                        targetLang, 6);

  if (!readTargetLang) return false;

  MicrosoftTranslator::prepareRequest(session, targetLang);
  MicrosoftTranslator::prepareRequestBody(body, text);

  session.SetBody(body);
  response = session.Post();

  return MicrosoftTranslator::parseResult(response,
                                          buffer,
                                          bufferSize,
                                          buffer,
                                          bufferSize);
}

bool readConfigValue(const char* section,
                     const char* item,
                     const char* defaultValue,
                     char* buffer, size_t bufferSize) {
  CSimpleIniA ini;
  iniConfig(ini);

  const char *read = ini.GetValue(section, item, defaultValue);

  if (read == nullptr) {
    *buffer = '\0';
  }
  else {
    strcpy_s(buffer, bufferSize, read);
  }

  return true;
}

bool writeConfigValue(const char* section, const char* item, const char* value) {
  CSimpleIniA ini;
  iniConfig(ini);
  SI_Error status = ini.SetValue(section, item, value);

  if (status != SI_UPDATED && status != SI_INSERTED) return false;

  wchar_t path[256];
  getConfigPath(path, 256);
  status = ini.SaveFile(path);

  return status == SI_OK;
}

namespace MicrosoftTranslator {

// TODO: void -> bool if config is fine otherwise somehow return error
void prepareRequest(cpr::Session& session, const char *targetLang) {
  static char* url = (char*)malloc(sizeof(char) * 128);
  static char* apiVersion = (char*)malloc(sizeof(char) * 32);
  static char* profanityAction = (char*)malloc(sizeof(char) * 64);
  static char* textType = (char*)malloc(sizeof(char) * 32);
  static char* rapidapiHost = (char*)malloc(sizeof(char) * 128);
  static char* rapidapiKey = (char*)malloc(sizeof(char) * 128);

  readConfigValue(MICROSOFT_SECTION, MS_URL, "", url, 128);
  readConfigValue(MICROSOFT_SECTION, MS_API_VERSION, "", apiVersion, 20);
  readConfigValue(MICROSOFT_SECTION, MS_PROFANITY_ACTION, "", profanityAction, 64);
  readConfigValue(MICROSOFT_SECTION, MS_TEXT_TYPE, "", textType, 32);
  readConfigValue(MICROSOFT_SECTION, MS_RAPIDAPI_HOST, "", rapidapiHost, 128);
  readConfigValue(MICROSOFT_SECTION, MS_RAPIDAPI_KEY, "", rapidapiKey, 128);

  session.SetUrl(cpr::Url{ url });
  session.SetParameters(
    cpr::Parameters{
      {"to", targetLang},
      {"apiVersion", apiVersion},
      {"profanityAction", profanityAction},
      {"textType", textType}
    });

  session.SetHeader(
    cpr::Header{ {"content-type",    "application/json"},
                 {"x-rapidapi-host", rapidapiHost},
                 {"x-rapidapi-key",  rapidapiKey}
    });
}

void prepareRequestBody(cpr::Body &body, const char* text) {
  Json::FastWriter stringifier;
  Json::Value data = Json::arrayValue;

  data[0] = Json::objectValue;
  data[0]["text"] = text;
  body = cpr::Body{ stringifier.write(data) };
}

bool parseResult(const cpr::Response & response,
                 char* resultBuffer,
                 size_t rbSize,
                 char* errorBuffer,
                 size_t ebSize) {
  Json::Value json;
  Json::Reader parser;

  if (!parser.parse(response.text, json)) {
    strcpy_s(errorBuffer, ebSize, "Received invalid JSON response.");
    return false;
  }

  if (response.status_code == 200L) {
    strcpy_s(resultBuffer, rbSize, json[0]["translations"][0]["text"].asCString());
    return true;
  }
  else {
    strcpy_s(errorBuffer, ebSize, json["error"]["message"].asCString());
    return false;
  }
}

}
