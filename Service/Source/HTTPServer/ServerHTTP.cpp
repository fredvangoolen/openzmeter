/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ServerHTTP.cpp
 * Author: edu
 * 
 * Created on 28 de marzo de 2019, 9:45
 */

#include "ServerHTTP.h"
using namespace httplib;
ServerHTTP::ServerHTTP() {
    Server svr;

    svr.Get("/hi", [](const Request& /*req*/, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.listen("localhost", 1234);

}

ServerHTTP::~ServerHTTP() {
}

