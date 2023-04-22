#include "mongoose.h"
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "am-duong-lich.h"
#include <unordered_map>
#include <array>
#include <regex>

#define URL_MAXLENGTH 2048
#define RESPONSE_HEADER "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n"
// ref: https://stackoverflow.com/questions/15491894/regex-to-validate-date-formats-dd-mm-yyyy-dd-mm-yyyy-dd-mm-yyyy-dd-mmm-yyyy
#define REGEX_VALID_DATE "^(0[1-9]|[12][0-9]|3[01])[/](0[1-9]|1[012])[/](19|20)\\d\\d$"
// ref: https://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
#define REGEX_FLOAT_NUMBER "[+-]?([0-9]*[.])?[0-9]+"

std::unordered_map<std::string, std::string> parseQuery(const mg_str &query);
nlohmann::json getResult(const int &dd, const int &mm, const int &yyyy, const double &timezones);
std::string getErrorMessage(const std::string &message);

std::array<int, 3> parseDate(const std::string &date);

// ref: https://www.geeksforgeeks.org/program-check-date-valid-not/
#define MAX_VALID_YR 9999
#define MIN_VALID_YR 1800
bool isLeap(int year);
bool isValidDate(int d, int m, int y);

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    std::cout << "Request to" << hm->method.ptr << std::endl;
    if (mg_http_match_uri(hm, "/api/day") && strncmp(hm->method.ptr, "GET", hm->method.len) == 0)
    {
      std::unordered_map<std::string, std::string> query = parseQuery(hm->query);
      if (query.find("query") == query.end())
      {
        mg_http_reply(c, 400, RESPONSE_HEADER, "%s\n", getErrorMessage("no query").c_str());
        return;
      }

      if (!std::regex_match(query.at("query"), std::regex(REGEX_VALID_DATE)))
      {
        mg_http_reply(c, 400, RESPONSE_HEADER, "%s\n", getErrorMessage("invalid query").c_str());
        return;
      }

      auto date = parseDate(query.at("query"));

      double timezones = 7.0;
      if (query.find("tz") != query.end())
      {
        if (!std::regex_match(query.at("tz"), std::regex(REGEX_FLOAT_NUMBER)))
        {
          mg_http_reply(c, 400, RESPONSE_HEADER, "%s\n", getErrorMessage("invalid timezones").c_str());
          return;
        }
        timezones = std::stod(query.at("tz"));
      }

      nlohmann::json result = getResult(date[0], date[1], date[2], timezones);
      mg_http_reply(c, 200, RESPONSE_HEADER, "%s\n", result.dump().c_str()); // Send dynamic JSON response
    }
    else
    {
      nlohmann::json endpoint;
      endpoint["message"] = "You\'re at the endpoint";
      endpoint["success"] = true;

      mg_http_reply(c, 200, RESPONSE_HEADER, "%s\n", endpoint.dump().c_str()); // For all other URIs,
    }
  }
}

int main(int argc, char *argv[])
{
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);                                     // Init manager
  mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, &mgr); // Setup listener
  for (;;)
    mg_mgr_poll(&mgr, 1000); // Event loop
  mg_mgr_free(&mgr);         // Cleanup
  return 0;
}

std::unordered_map<std::string, std::string> parseQuery(const mg_str &query)
{
  std::unordered_map<std::string, std::string> queries;

  if (query.len == 0)
  {
    return queries;
  }

  char queryStr[URL_MAXLENGTH];
  memset(queryStr, 0x0, URL_MAXLENGTH);
  strncpy(queryStr, query.ptr, query.len);

  std::string raw = queryStr;
  std::stringstream stream(raw);
  std::string token;
  while (std::getline(stream, token, '&'))
  {
    // std::cout << token << std::endl;
    std::string key = "";
    std::string value = "";
    bool isProcessKey = true;
    for (char &c : token)
    {
      if (isProcessKey && c == '=')
      {
        isProcessKey = false;
        continue;
      }

      if (isProcessKey)
        key += c;
      else
        value += c;
    }

    queries.insert({key, value});
  }

  return queries;
}

nlohmann::json getResult(const int &dd, const int &mm, const int &yyyy, const double &timezones)
{
  auto lunar = convertToLunarCalendar(dd, mm, yyyy, timezones);
  nlohmann::json result;
  result["solar"]["day"] = dd;
  result["solar"]["month"] = mm;
  result["solar"]["year"] = yyyy;
  result["timezones"] = timezones;
  result["lunar"]["day"] = lunar[0];
  result["lunar"]["month"] = lunar[1];
  result["lunar"]["year"] = lunar[2];
  result["lunar"]["leap"] = (bool)lunar[3];

  return result;
}

std::string getErrorMessage(const std::string &message)
{
  nlohmann::json m;
  m["message"] = message;

  return m.dump();
}

std::array<int, 3> parseDate(const std::string &date)
{
  std::array<int, 3> formatted;
  std::stringstream stream(date);
  std::string token;

  // get date
  std::getline(stream, token, '/');
  formatted[0] = std::stoi(token);

  // get month
  std::getline(stream, token, '/');
  formatted[1] = std::stoi(token);

  // get year
  std::getline(stream, token, '/');
  formatted[2] = std::stoi(token);

  return formatted;
}

bool isLeap(int year)
{
  return (((year % 4 == 0) &&
           (year % 100 != 0)) ||
          (year % 400 == 0));
}

bool isValidDate(int d, int m, int y)
{
  // If year, month and day
  // are not in given range
  if (y > MAX_VALID_YR ||
      y < MIN_VALID_YR)
    return false;
  if (m < 1 || m > 12)
    return false;
  if (d < 1 || d > 31)
    return false;

  // Handle February month
  // with leap year
  if (m == 2)
  {
    if (isLeap(y))
      return (d <= 29);
    else
      return (d <= 28);
  }

  // Months of April, June,
  // Sept and Nov must have
  // number of days less than
  // or equal to 30.
  if (m == 4 || m == 6 ||
      m == 9 || m == 11)
    return (d <= 30);

  return true;
}