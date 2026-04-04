#include "../exchange/dummy_exchange_server.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void applyArg(DummyExchangeConfig& config, const std::string& key, const std::string& value) {
    if (key == "--listen-ip") {
        config.listen_ip = value;
    } else if (key == "--port") {
        config.port = static_cast<uint16_t>(std::stoul(value));
    } else if (key == "--username") {
        config.username = value;
    } else if (key == "--password") {
        config.password = value;
    } else if (key == "--session") {
        config.session_id = value;
    } else if (key == "--price-min") {
        config.price_min = static_cast<uint32_t>(std::stoul(value));
    } else if (key == "--price-max") {
        config.price_max = static_cast<uint32_t>(std::stoul(value));
    } else if (key == "--max-shares") {
        config.max_shares = static_cast<uint32_t>(std::stoul(value));
    } else if (key == "--fill-delay-ms") {
        config.fill_delay = std::chrono::milliseconds(std::stol(value));
    } else {
        throw std::invalid_argument("unknown argument: " + key);
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        DummyExchangeConfig config {};
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc) {
                throw std::invalid_argument("missing value for argument");
            }
            applyArg(config, argv[i], argv[i + 1]);
        }

        DummyExchangeServer server(config);
        return server.run();
    } catch (const std::exception& ex) {
        std::cerr << "dummy_exchange_server error: " << ex.what() << '\n';
        return 1;
    }
}
