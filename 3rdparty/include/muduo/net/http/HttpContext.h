// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_HTTP_HTTPCONTEXT_H
#define MUDUO_NET_HTTP_HTTPCONTEXT_H

#include <muduo/base/copyable.h>

#include <muduo/net/http/HttpRequest.h>

#include <boost/shared_array.hpp>
#include <boost/function.hpp>

namespace muduo
{
namespace net
{

class Buffer;

class HttpContext : public muduo::copyable
{
 public:
  typedef boost::function<bool(Buffer*,Timestamp)> RequestParser;

  enum HttpRequestParseState
  {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext()
    : state_(kExpectRequestLine),postdata_()
  {
  }

  // default copy-ctor, dtor and assignment are fine
  virtual ~HttpContext(){}

  // return false if any error
  bool parseRequest(Buffer* buf, Timestamp receiveTime);

  bool gotAll() const
  { return state_ == kGotAll; }

  void reset()
  {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const
  { return request_; }

  HttpRequest& request()
  { return request_; }

  void setRequestParser(const RequestParser& parser){
    requestParser_ = parser;
  }
  const RequestParser& getRequestParser() const{
    return requestParser_;
  }
  HttpRequestParseState& getState(){
    return state_;
  }
  boost::shared_array<char>& getPostData(){
    return postdata_;
  }

  bool processRequestLine(const char* begin, const char* end);
private:
  HttpRequestParseState state_;
  HttpRequest request_;
  boost::shared_array<char> postdata_; // 如果希望深拷贝,这里将出现问题;也就是多个处理线程间不应该共享HttpContext,如果要支持,设置成线程局部变量
  RequestParser requestParser_;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPCONTEXT_H
