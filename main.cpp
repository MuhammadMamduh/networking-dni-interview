#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include "httplib.h"

// Global cache for resolved stream URLs
std::map<std::string, std::string> stream_cache;
std::mutex cache_mutex;

// Helper function to extract URL via Streamlink
std::string get_raw_url(const std::string& channel) {
    std::string cmd = "streamlink --stream-url https://www.twitch.tv/" + channel + " best 2>/dev/null";
    char buffer[256];
    std::string result = "";
    
    // This is a pointer to a file stream.
    // Instead of pointing to a file on your hard drive, this pointer acts as a "window" into the live output of the Streamlink process.
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "Error";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);

    std::cout << "$result -> " << result <<std::endl;

    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> channels = {"quran_live24", "alquran4k"}; // Example

    // 1. "Pre-fetch" phase: resolve URLs before the server opens
    std::cout << "Resolving stream URLs... please wait." << std::endl;
    for(const auto& c : channels) {
        std::string url = get_raw_url(c);
        std::lock_guard<std::mutex> lock(cache_mutex);
        stream_cache[c] = url;
    }

    httplib::Server svr;
    
    // 2. The Request Handler: Lightning fast because it only reads from memory
    // 2. The Request Handler: Now includes an embedded HLS player
    svr.Get("/", [&](const httplib::Request&, httplib::Response& res) {
        std::string html = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>C++ Stream Player</title>
                <script src="https://cdn.jsdelivr.net/npm/hls.js@latest"></script>
                <style>
                    body { background: #111; color: white; font-family: sans-serif; text-align: center; }
                    .video-container { margin-bottom: 30px; padding: 20px; border-bottom: 1px solid #333; }
                    video { width: 80%; max-width: 800px; background: black; border: 2px solid #444; }
                </style>
            </head>
            <body>
                <h1>Live Stream Dashboard</h1>
        )";

        std::lock_guard<std::mutex> lock(cache_mutex);
        int count = 0;
        for(const auto& pair : stream_cache) {
            std::string vidId = "video" + std::to_string(count);
            html += "<div class='video-container'>";
            html += "<h3>Channel: " + pair.first + "</h3>";
            html += "<video id='" + vidId + "' controls autoplay muted></video>";
            html += "<script>";
            html += "  if (Hls.isSupported()) {";
            html += "    var video = document.getElementById('" + vidId + "');";
            html += "    var hls = new Hls();";
            html += "    hls.loadSource('" + pair.second + "');";
            html += "    hls.attachMedia(video);";
            html += "  } else if (video.canPlayType('application/vnd.apple.mpegurl')) {";
            html += "    video.src = '" + pair.second + "';"; // Native support for Safari
            html += "  }";
            html += "</script></div>";
            count++;
        }
        
        html += "</body></html>";
        res.set_content(html, "text/html");
    });

    std::cout << "Server live at http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}