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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace hw4 {

    const int bufSize = 1024;
    static const char *kHeaderEnd = "\r\n\r\n";
    static const int kHeaderEndLen = 4;

    bool HttpConnection::GetNextRequest(HttpRequest *request) {
        // Use "WrappedRead" to read data into the buffer_
        // instance variable.  Keep reading data until either the
        // connection drops or you see a "\r\n\r\n" that demarcates
        // the end of the request header. Be sure to try and read in
        // a large amount of bytes each time you call WrappedRead.
        //
        // Once you've seen the request header, use ParseRequest()
        // to parse the header into the *request argument.
        //
        // Very tricky part:  clients can send back-to-back requests
        // on the same socket.  So, you need to preserve everything
        // after the "\r\n\r\n" in buffer_ for the next time the
        // caller invokes GetNextRequest()!

        // STEP 1:
        size_t pos = buffer_.find(kHeaderEnd);
        if (pos == std::string::npos) {
            int byte_read;
            unsigned char buf[bufSize];
            while (true) {
                byte_read = WrappedRead(fd_, buf, bufSize);
                if (byte_read == 0) {
                    break;
                } else if (byte_read == -1) {
                    return false;
                }
                buffer_ += string
                        (reinterpret_cast<char*>(buf), byte_read);
                pos = buffer_.find(kHeaderEnd);
                if (pos != std::string::npos)
                    break;
            }
        }
        if (pos == std::string::npos) {
            return false;
        }
        *request = ParseRequest(buffer_.substr(0, pos + kHeaderEndLen));
        // preserve the buffer
        buffer_ = buffer_.substr(pos + kHeaderEndLen);

        return true;
    }

    bool HttpConnection::WriteResponse(const HttpResponse &response) {
        string str = response.GenerateResponseString();
        int res = WrappedWrite(fd_,
                               (unsigned char *) str.c_str(),
                               str.length());
        if (res != static_cast<int>(str.length()))
            return false;
        return true;
    }

    HttpRequest HttpConnection::ParseRequest(const string &request) {
        HttpRequest req("/");  // by default, get "/".

        // Split the request into lines.  Extract the URI from the first line
        // and store it in req.URI.  For each additional line beyond the
        // first, extract out the header name and value and store them in
        // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
        // at HttpRequest.h for details about the HTTP header format that
        // you need to parse.
        //
        // You'll probably want to look up boost functions for (a) splitting
        // a string into lines on a "\r\n" delimiter, (b) trimming
        // whitespace from the end of a string, and (c) converting a string
        // to lowercase.
        //
        // Note that you may assume the request you are parsing is correctly
        // formatted. If for some reason you encounter a header that is
        // malformed, you may skip that line.

        // STEP 2:
        if (request.empty())
            return req;

        // split request into lines
        vector<std::string> lines;
        boost::iter_split(lines, request,
                boost::algorithm::first_finder("\r\n"));
        for (string str : lines) {
            boost::trim(str);
        }

        // set URI from first line
        vector<std::string> tokens;
        boost::split(tokens, lines[0], boost::is_any_of(" "));
        req.set_uri(tokens[1]);

        // add headers
        for (size_t i = 1; i < lines.size(); i++) {
            if (lines[i].size() != 0) {
                size_t index = lines[i].find(": ");
                if (index == std::string::npos) {
                    continue;
                }
                tokens.clear();
                boost::split(tokens, lines[i], boost::is_any_of(": "),
                             boost::token_compress_on);
                boost::to_lower(tokens[0]);
                req.AddHeader(tokens[0], tokens[1]);
            }
        }
        return req;
    }

}  // namespace hw4
