#pragma once

constexpr auto main_template = R"cpp(
#include <smartview.hpp>
#include <serializers/json.hpp>

int main()
{
    saucer::simple_smartview<saucer::serializers::json> webview;

    webview.set_title("Hello World!");
    webview.set_url("https://ddg.gg");
    webview.set_size(500, 600);
    webview.show();
    webview.run();

    return 0;
}
)cpp";