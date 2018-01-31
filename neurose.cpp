#include <stdlib.h>
#include <node.h>
#include "request.h"
#include "hiredis.h"

#include <string.h>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

redisContext *c=NULL;

int request_num=0;
  
void connect_redis(const FunctionCallbackInfo<Value>& args) 
{
	char buffer[1024];
	char*result=( char*)&buffer;
	Isolate* isolate = args.GetIsolate();
	v8::String::Utf8Value arg0(args[0]);
	const char *hostname = *arg0;
	int port;
	if(args[1]->IsNumber())
	{
		port=args[1]->NumberValue();
	}else
		port=7777;
	
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	strcpy(result,"Connected to redis");
    c = redisConnectWithTimeout(hostname, port, timeout);
	if (c == NULL || c->err) {
		if (c) {
			sprintf(result,"Connection error: %s", c->errstr);
			redisFree(c);
		} else {
			strcpy(result,"Connection error: can't allocate redis context");
		}
	}
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
	
}
void ping_redis(const FunctionCallbackInfo<Value>& args) 
{
	char buffer[1024];
	char*result=( char*)&buffer;
	Isolate* isolate = args.GetIsolate();
	
	if(c)
	{
	    redisReply *reply;

	    reply = (redisReply *)redisCommand(c,"PING");
	    sprintf(result,"PING: %s", reply->str);
	    freeReplyObject(reply);
	}else{
		strcpy(result,"PING: error -> not connected!");
		
	}
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
	
}
void disconnect_redis(const FunctionCallbackInfo<Value>& args) 
{
	char buffer[1024];
	char*result=( char*)&buffer;
	Isolate* isolate = args.GetIsolate();
	redisFree(c);
	c=NULL;
	strcpy(result,"Disconnected from redis");
	
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
}

void work(const FunctionCallbackInfo<Value>& args)
{
	static char *buffer = NULL;
	int ret=0;
	const char*result=(const char*)&buffer;
	Isolate* isolate = args.GetIsolate();
	v8::String::Utf8Value arg0(args[0]);
	v8::String::Utf8Value arg1(args[1]);
	v8::String::Utf8Value arg2(args[2]);
	const char *method = *arg0;
	const char *url    = *arg1;
	const char *body   = *arg2;
	int validate=0;
    if(!buffer)
	{
		buffer=(char*)malloc(sizeof(char) * 91024);
		*buffer = '\0';
	}else{
		*buffer = '\0';
	}
	if(args[3]->IsNumber())
	{
		validate=args[3]->NumberValue();
	}else
		validate=0;

	//printf("validate[%d] [%s] %s {%s}\n",validate,method,url,body);

	ret=handle_request(c,method,url,body,validate,buffer);
	/*sprintf(msg,"<head><title>GWF Basic Page</title></head><body><h1>request %d</h1><h2>method-%s</h2><br><h2>url %s</h2><br><h2>body %s</h2><br></body>",
	++request_num,
	method,url,body);*/
	switch(ret)
	{
		case -1:
			strcpy((char*)buffer,"erro processando metodo");
			break;
		case -2:
			strcpy((char*)buffer,"erro processando url");
			break;
		case -3:
			strcpy((char*)buffer,"erro processando json");
			break;
	}


	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
}

void Init(Local<Object> exports) {
	NODE_SET_METHOD(exports, "connect_redis", connect_redis);
	NODE_SET_METHOD(exports, "disconnect_redis", disconnect_redis);
	NODE_SET_METHOD(exports, "ping_redis", ping_redis);
	NODE_SET_METHOD(exports, "work", work);
}

NODE_MODULE(cpphello, Init)
