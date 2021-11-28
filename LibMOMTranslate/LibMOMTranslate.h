#include "framework.h"

#ifdef LIBMOMTRANSLATE_EXPORTS
#define LIBMOMTRANSLATE_API __declspec(dllexport)
#else
#define LIBMOMTRANSLATE_API __declspec(dllimport)
#endif

#define TRANSLATOR_SECTION          "translator"
#define TRANSLATOR_TARGET_LANG_KEY  "target_lang"

#define MICROSOFT_SECTION           "microsoft"

#define MS_URL                      "url"
#define MS_API_VERSION              "api_version"
#define MS_PROFANITY_ACTION         "profanity_action"
#define MS_TEXT_TYPE                "text_type"

#define MS_RAPIDAPI_HOST            "rapidapi_host"
#define MS_RAPIDAPI_KEY             "rapidapi_key"

#define FOR_TESTS_SECTION           "for-tests"
#define FOR_TESTS_KEY               "test-key"


extern "C"
LIBMOMTRANSLATE_API bool translate(const char* text,
                                   char* buffer,
                                   size_t bufferSize);

extern "C"
LIBMOMTRANSLATE_API bool readConfigValue(const char* section,
                                         const char* item,
                                         const char* defaultValue,
                                         char* buffer,
                                         size_t bufferSize);

extern "C"
LIBMOMTRANSLATE_API bool writeConfigValue(const char* section,
                                          const char* item,
                                          const char* value);

namespace MicrosoftTranslator {

void prepareRequest(cpr::Session& session, const char *targetlang);
void prepareRequestBody(cpr::Body& body, const char *text);
bool parseResult(const cpr::Response& response,
                 char* resultBuffer,
                 size_t rbSize,
                 char *errorBuffer,
                 size_t ebSize);

}

