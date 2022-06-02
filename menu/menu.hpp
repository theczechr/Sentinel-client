#pragma once
#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <botan/bcrypt.h>
#include <botan/auto_rng.h>
#include "server.hpp"

namespace menu
{
	void start();
	void login();
	void create();
}
