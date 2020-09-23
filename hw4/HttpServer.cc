/*
 * Copyright ©2020 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Summer Quarter 2020 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include "./FileReader.h"
#include "./HttpConnection.h"
#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpServer.h"
#include "./libhw3/QueryProcessor.h"

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

namespace hw4 {
///////////////////////////////////////////////////////////////////////////////
// Constants, internal helper functions
///////////////////////////////////////////////////////////////////////////////
static const char *kThreegleStr =
  "<html><head><title>333gle</title></head>\n"
  "<body>\n"
  "<center style=\"font-size:500%;\">\n"
  "<span style=\"position:relative;bottom:-0.33em;color:orange;\">3</span>"
    "<span style=\"color:red;\">3</span>"
    "<span style=\"color:gold;\">3</span>"
    "<span style=\"color:blue;\">g</span>"
    "<span style=\"color:green;\">l</span>"
    "<span style=\"color:red;\">e</span>\n"
  "</center>\n"
  "<p>\n"
  "<div style=\"height:20px;\"></div>\n"
  "<center>\n"
  "<form action=\"/query\" method=\"get\">\n"
  "<input type=\"text\" size=30 name=\"terms\" />\n"
  "<input type=\"submit\" value=\"Search\" />\n"
  "</form>\n"
  "</center><p>\n";

// static
const int HttpServer::kNumThreads = 100;

// This is the function that threads are dispatched into
// in order to process new client connections.
static void HttpServer_ThrFn(ThreadPool::Task *t);

// Given a request, produce a response.
static HttpResponse ProcessRequest(const HttpRequest &req,
                            const string &basedir,
                            const list<string> *indices);

// Process a file request.
static HttpResponse ProcessFileRequest(const string &uri,
                                const string &basedir);

// Process a query request.
static HttpResponse ProcessQueryRequest(const string &uri,
                                 const list<string> *indices);


///////////////////////////////////////////////////////////////////////////////
// HttpServer
///////////////////////////////////////////////////////////////////////////////
bool HttpServer::Run(void) {
  // Create the server listening socket.
  int listen_fd;
  cout << "  creating and binding the listening socket..." << endl;
  if (!ss_.BindAndListen(AF_INET6, &listen_fd)) {
    cerr << endl << "Couldn't bind to the listening socket." << endl;
    return false;
  }

  // Spin, accepting connections and dispatching them.  Use a
  // threadpool to dispatch connections into their own thread.
  cout << "  accepting connections..." << endl << endl;
  ThreadPool tp(kNumThreads);
  while (1) {
    HttpServerTask *hst = new HttpServerTask(HttpServer_ThrFn);
    hst->basedir = staticfileDirpath_;
    hst->indices = &indices_;
    if (!ss_.Accept(&hst->client_fd,
                    &hst->caddr,
                    &hst->cport,
                    &hst->cdns,
                    &hst->saddr,
                    &hst->sdns)) {
      // The accept failed for some reason, so quit out of the server.
      // (Will happen when kill command is used to shut down the server.)
      break;
    }
    // The accept succeeded; dispatch it.
    tp.Dispatch(hst);
  }
  return true;
}

static void HttpServer_ThrFn(ThreadPool::Task *t) {
  // Cast back our HttpServerTask structure with all of our new
  // client's information in it.
  unique_ptr<HttpServerTask> hst(static_cast<HttpServerTask *>(t));
  cout << "  client " << hst->cdns << ":" << hst->cport << " "
       << "(IP address " << hst->caddr << ")" << " connected." << endl;

  // Read in the next request, process it, write the response.

  // Use the HttpConnection class to read and process the next
  // request from our current client, then write out our response.  If
  // the client sends a "Connection: close\r\n" header, then shut down
  // the connection -- we're done.
  //
  // Hint: the client can make multiple requests on our single connection,
  // so we should keep the connection open between requests rather than
  // creating/destroying the same connection repeatedly.

  // STEP 1:
  bool done = false;
  HttpConnection connection(hst->client_fd);
  while (!done) {
    HttpRequest request;
    // get next request
    if (!connection.GetNextRequest(&request)) {
      done = true;
      break;
    }
    // process the request
    HttpResponse response = ProcessRequest(request, hst->basedir, hst->indices);
    if (!connection.WriteResponse(response)) {
      done = true;
    }
    // check header
    if (request.GetHeaderValue("connection").compare("close") == 0) {
      done = true;
    }
  }
}

static HttpResponse ProcessRequest(const HttpRequest &req,
                            const string &basedir,
                            const list<string> *indices) {
  // Is the user asking for a static file?
  if (req.uri().substr(0, 8) == "/static/") {
    return ProcessFileRequest(req.uri(), basedir);
  }

  // The user must be asking for a query.
  return ProcessQueryRequest(req.uri(), indices);
}

static HttpResponse ProcessFileRequest(const string &uri,
                                const string &basedir) {
  // The response we'll build up.
  HttpResponse ret;

  // Steps to follow:
  //  - use the URLParser class to figure out what filename
  //    the user is asking for.
  //
  //  - use the FileReader class to read the file into memory
  //
  //  - copy the file content into the ret.body
  //
  //  - depending on the file name htype, set the response
  //    Content-type header as appropriate, e.g.,:
  //      --> for ".html" or ".htm", set to "text/html"
  //      --> for ".jpeg" or ".jpg", set to "image/jpeg"
  //      --> for ".png", set to "image/png"
  //      etc.
  //
  // be sure to set the response code, protocol, and message
  // in the HttpResponse as well.
  string fname = "";

  // STEP 2:
  URLParser parser;
  parser.Parse(uri);
  // delete "/static/"
  fname = parser.path().substr(8);

  FileReader freader(basedir, fname);
  string content;
  if (freader.ReadFile(&content)) {
    ret.set_response_code(200);
    ret.set_protocol("HTTP/1.1");
    ret.set_message("OK");

    ret.AppendToBody(content);

    vector<std::string> tokens;
    boost::split(tokens, fname, boost::is_any_of("."));
    string htype = tokens[1];

    if (htype.compare("html") == 0 || htype.compare("htm") == 0)
      ret.set_content_type("text/html");
    else if (htype.compare("csv") == 0)
      ret.set_content_type("text/csv");
    else if (htype.compare("css") == 0)
      ret.set_content_type("text/css");
    else if (htype.compare("ics") == 0)
      ret.set_content_type("text/calendar");
    else if (htype.compare("js") == 0)
      ret.set_content_type("text/javascript");
    else if (htype.compare("txt") == 0)
      ret.set_content_type("text/plain");
    else if (htype.compare("jpg") == 0 || htype.compare("jpeg") == 0)
      ret.set_content_type("image/jpeg");
    else if (htype.compare("gif") == 0)
      ret.set_content_type("image/gif");
    else if (htype.compare("png") == 0)
      ret.set_content_type("image/png");
    else if (htype.compare("tiff") == 0)
      ret.set_content_type("image/tiff");
    else if (htype.compare("xml") == 0)
      ret.set_content_type("text/xml");
    else if (htype.compare("svg") == 0)
      ret.set_content_type("image/svg+xml");
    else
      ret.set_content_type("application/octet-stream");

    return ret;
  }

  // If you couldn't find the file, return an HTTP 404 error.
  ret.set_protocol("HTTP/1.1");
  ret.set_response_code(404);
  ret.set_message("Not Found");
  ret.AppendToBody("<html><body>Couldn't find file \""
                   + EscapeHtml(fname)
                   + "\"</body></html>");
  return ret;
}

static HttpResponse ProcessQueryRequest(const string &uri,
                                 const list<string> *indices) {
  // The response we're building up.
  HttpResponse ret;

  // Your job here is to figure out how to present the user with
  // the same query interface as our solution_binaries/http333d server.
  // A couple of notes:
  //
  //  - no matter what, you need to present the 333gle logo and the
  //    search box/button
  //
  //  - if the user had previously typed in a search query, you also
  //    need to display the search results.
  //
  //  - you'll want to use the URLParser to parse the uri and extract
  //    search terms from a typed-in search query.  convert them
  //    to lower case.
  //
  //  - you'll want to create and use a hw3::QueryProcessor to process
  //    the query against the search indices
  //
  //  - in your generated search results, see if you can figure out
  //    how to hyperlink results to the file contents, like we did
  //    in our solution_binaries/http333d.

  // STEP 3:

  // present the 333gle logo and the search box/button
  ret.AppendToBody(kThreegleStr);

  URLParser p;
  p.Parse(uri);
  string terms = EscapeHtml(p.args()["terms"]);
  if (uri.find("query?terms=") != string::npos) {
    boost::trim(terms);
    boost::to_lower(terms);
    hw3::QueryProcessor qp(*indices);
    vector<string> queryTerms;
    boost::split(queryTerms, terms, boost::is_any_of(" "));
    vector<hw3::QueryProcessor::QueryResult> results = qp.ProcessQuery
        (queryTerms);

    // display the searching result
    ret.AppendToBody("<p><br>\n");
    if (results.empty()) {
      ret.AppendToBody("No results found for <b>");
    } else {
      ret.AppendToBody(std::to_string(results.size()));
      ret.AppendToBody(" results found for <b>");
    }
    ret.AppendToBody(terms);
    ret.AppendToBody("</b>\n<p>\n\n");
    ret.AppendToBody("<ul>\n");

    // hyperlink the results
    for (auto result : results) {
      ret.AppendToBody(" <li> <a href=\"");
      if (result.documentName.substr(0, 7).compare("http://") != 0) {
        ret.AppendToBody("/static/");
      }
      ret.AppendToBody(result.documentName);  // hyperlink
      ret.AppendToBody("\">");
      ret.AppendToBody(EscapeHtml(result.documentName));  // display name
      ret.AppendToBody("</a> [");
      ret.AppendToBody(std::to_string(result.rank));
      ret.AppendToBody("]<br>\n");
    }
    ret.AppendToBody("</ul>\n");
  }
  ret.AppendToBody("</body>\n</html>\n");

  // set the response code, protocol, and message
  ret.set_protocol("HTTP/1.1");
  ret.set_response_code(200);
  ret.set_message("OK");
  return ret;
}
}  // namespace hw4
