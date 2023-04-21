#include "mongoose.h"
#include <nlohmann/json.hpp>
#include "am-duong-lich.h"

#define RESPONSE_HEADER "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n"

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    
    printf("Request to %s", hm->method.ptr);
    if (mg_http_match_uri(hm, "/api/day") && strncmp(hm->method.ptr, "GET", hm->method.len) == 0)
    {
      printf("QUERY=%s", hm->query.ptr);
      auto lunar = convertToLunarCalendar(12, 04, 2000, 07);
      nlohmann::json result;
      result["day"] = lunar[0];
      result["month"] = lunar[1];
      result["year"] = lunar[2];
      result["leap"] = (bool)lunar[3];
      mg_http_reply(c, 200, RESPONSE_HEADER, "%s\n", result.dump().c_str());          // Send dynamic JSON response
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
