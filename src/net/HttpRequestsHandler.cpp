#include "HttpRequestsHandler.h"

#include <GSON.h>

#include "res/Titles.h"

#define JSON_CONTENT "application/json"
#define PARAM_SERIAL "serial"

#define CODE_200 200
#define CODE_404 404

namespace
{
    bool hasSerialParam(AsyncWebServerRequest *request, String &out)
    {
        if (request->hasParam(PARAM_SERIAL))
        {
            out = request->getParam(PARAM_SERIAL)->value();
            return true;
        }
        return false;
    }

    void sendJsonResponse(AsyncWebServerRequest *request, int code, gson::Str &gs)
    {
        auto response = request->beginResponseStream(JSON_CONTENT);
        response->setCode(code);
        response->print(gs);
        request->send(response);
    }

    void sendSuccessResponse(AsyncWebServerRequest *request, gson::Str &gs)
    {
        sendJsonResponse(request, CODE_200, gs);
    }

    void sendJsonError(AsyncWebServerRequest *const request, int code, const String &message)
    {
        gson::Str gs;
        gs('{');
        gs["status"] = "error";
        gs["code"] = code;
        gs["message"] = message;
        gs('}');
        sendJsonResponse(request, code, gs);
    }

    void sendError404(AsyncWebServerRequest *request)
    {
        sendJsonError(request, CODE_404, UiTitles::Messages::DEVICE_NOT_FOUND);
    }

    template<typename Handler>
    void withSerialParam(AsyncWebServerRequest *request, Handler handler)
    {
        String stateMachineId;
        if (hasSerialParam(request, stateMachineId))
        {
            gson::Str doc;
            doc('{');
            handler(stateMachineId, doc);
            doc('}');
            sendSuccessResponse(request, doc);
        }
        else
        {
            sendError404(request);
        }
    }
}

HttpRequestsHandler::HttpRequestsHandler(
    AsyncWebServer *const s,
    CommandsHandler *const h) : server(s), handler(h), printer() {}

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
    withSerialParam(request, [this](const String &stateMachineId, gson::Str &doc) {
        handler->handleStatus(stateMachineId, statusCb, doc);
    });
}

void HttpRequestsHandler::handleStart(AsyncWebServerRequest *const request)
{
    withSerialParam(request, [this](const String &stateMachineId, gson::Str &doc) {
        handler->handleStart(stateMachineId, doc);
    });
}

void HttpRequestsHandler::handleStop(AsyncWebServerRequest *const request)
{
    withSerialParam(request, [this](const String &stateMachineId, gson::Str &doc) {
        handler->handleStop(stateMachineId, doc);
    });
}

void HttpRequestsHandler::handleAll(AsyncWebServerRequest *const request)
{
    gson::Str doc;
    doc('{');
    handler->handleAll(doc);
    doc('}');
    sendSuccessResponse(request, doc);
}

void HttpRequestsHandler::print404(AsyncWebServerRequest *const request)
{
    sendError404(request);
}