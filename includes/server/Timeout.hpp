// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Timeout.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "EventHandler.hpp"
#include <unordered_map>

class Timeout : public EventHandler{
    private:
        int _timeFd;
        std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds;
        Timeout() = default;
    public:
        Timeout(std::unordered_map<int*, std::vector<EventHandler*>>& fds);
        ~Timeout();

        int handleEvent(uint32_t ev) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        int* getSocketFd(void) override;
        void resolveClose() override;
};