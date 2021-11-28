#include <gtest/gtest.h>
#include <cstdio>
#include <string.h>

#include "pch.h"

#include "../LibMOMTranslate/LibMOMTranslate.h"

using std::string;

TEST(Simple, TranslateHello) {
  char hello[100];
  bool success = translate("Hello", hello, 100);

  ASSERT_TRUE(success);
  ASSERT_EQ((string) hello, "Привет");
}

TEST(readConfigValue, ReadTargetLanguage) {
  char tlBuffer[100];
  bool success = readConfigValue(TRANSLATOR_SECTION,
                                 TRANSLATOR_TARGET_LANG_KEY,
                                 nullptr,
                                 tlBuffer, 100);
  
  ASSERT_TRUE(success);
  ASSERT_EQ((string)tlBuffer, "ru");
}

TEST(readConfigValue, GetDefaultOnNonExistingKey) {
  char tlBuffer[100];
  bool success = readConfigValue(FOR_TESTS_SECTION,
                                 "9023fjk320",
                                 "default",
                                 tlBuffer, 100);
  ASSERT_TRUE(success);
  ASSERT_EQ((string)tlBuffer, "default");
}

TEST(readConfigValue, GetNullDefault) {
  char tlBuffer[100];
  bool success = readConfigValue(FOR_TESTS_SECTION,
                                 "9023fjk320",
                                 nullptr,
                                 tlBuffer, 100);
  ASSERT_TRUE(success);
  ASSERT_EQ((string) tlBuffer, "\0");
}

TEST(Simple, WriteGarbageKey) {
  CSimpleIniA ini;
  char value[16] = "3.1415926";
  char written[16];

  bool writeSuccess = writeConfigValue(FOR_TESTS_SECTION, FOR_TESTS_KEY, value);
  bool readSuccess = readConfigValue(FOR_TESTS_SECTION, FOR_TESTS_KEY, nullptr, written, 16);

  ASSERT_TRUE(writeSuccess);
  ASSERT_TRUE(readSuccess);
  ASSERT_EQ((string) written, (string) value);
}

