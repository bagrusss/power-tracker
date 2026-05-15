#include "HttpRequestsHandler.h"

#include <GSON.h>

#include "res/Titles.h"
#include "net/JsonKeys.h"
#include "util/StrUtils.h"

#define CODE_200 200
#define CODE_404 404

static bool hasSerialParam(AsyncWebServerRequest *request, String &out)
{
    const char *paramName = StrUtils::read(HTTP::PARAM_SERIAL);
    if (request->hasParam(paramName))
    {
        out = request->getParam(paramName)->value();
        return true;
    }
    return false;
}

static void sendJsonResponse(AsyncWebServerRequest *request, int code, gson::Str &gs)
{
    auto response = request->beginResponseStream(StrUtils::read(HTTP::CONTENT_TYPE));
    response->setCode(code);
    response->print(gs);
    request->send(response);
}

static void sendSuccessResponse(AsyncWebServerRequest *request, gson::Str &gs)
{
    sendJsonResponse(request, CODE_200, gs);
}

static void sendJsonError(AsyncWebServerRequest *const request, int code, const String &message, gson::Str &gs)
{
    gs('{');
    gs[JKEY::STATUS] = JVAL::ERROR;
    gs[JKEY::CODE] = code;
    gs[JKEY::MESSAGE] = message;
    gs('}');
    sendJsonResponse(request, code, gs);
}

static void sendError404(AsyncWebServerRequest *request, gson::Str &gs)
{
    sendJsonError(request, CODE_404, StrUtils::read(UiTitles::Messages::DEVICE_NOT_FOUND), gs);
}

template<typename Handler>
static void withSerialParam(AsyncWebServerRequest *request, Handler handler, gson::Str &gs)
{
    String stateMachineId;
    if (hasSerialParam(request, stateMachineId))
    {
        gs('{');
        handler(stateMachineId, gs);
        gs('}');
        sendSuccessResponse(request, gs);
    }
    else
    {
        sendError404(request, gs);
    }
}

HttpRequestsHandler::HttpRequestsHandler(
    AsyncWebServer *const s,
    CommandsHandler *const h) : server(s), handler(h), printer(6144) {}

void HttpRequestsHandler::begin()
{
    server->on("/status", WebRequestMethod::HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleStatus(request); });
    server->on("/start", WebRequestMethod::HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleStart(request); });
    server->on("/stop", WebRequestMethod::HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleStop(request); });
    server->on("/all", WebRequestMethod::HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleAll(request); });
}

void HttpRequestsHandler::setStatusCallback(StatusCallback cb)
{
    this->statusCb = cb;
}

void HttpRequestsHandler::handleStatus(AsyncWebServerRequest *const request)
{
    printer.clear();
    withSerialParam(request, [this](const String &stateMachineId, gson::Str &doc) {
        handler->handleStatus(stateMachineId, statusCb, doc);
    }, printer);
}

void HttpRequestsHandler::handleStart(AsyncWebServerRequest *const request)
{
    printer.clear();
    withSerialParam(request, [this](const String &stateMachineId, gson::Str &doc) {
        handler->handleStart(stateMachineId, doc);
    }, printer);
}

void HttpRequestsHandler::handleStop(AsyncWebServerRequest *const request)
{
    printer.clear();
    withSerialParam(request, [this](const String &stateMachineId, gson::Str &doc) {
        handler->handleStop(stateMachineId, doc);
    }, printer);
}

void HttpRequestsHandler::handleAll(AsyncWebServerRequest *const request)
{
    printer.clear();
    printer('{');
    handler->handleAll(printer);
    printer('}');
    sendSuccessResponse(request, printer);
}

void HttpRequestsHandler::print404(AsyncWebServerRequest *const request)
{
    printer.clear();
    sendError404(request, printer);
}
