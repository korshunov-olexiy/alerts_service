#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <json/json.h>
#include <gtkmm.h>
#include <gstreamermm.h>

std::string region;
std::string alert_on;
std::string alert_off;
std::string data_url;
int update_interval;

// alert_active - set true if warning activate
bool alert_active = false;

/**
 * @brief WriteCallback function to handle writing data from a callback function.
 * @param contents void pointer to the data contents
 * @param size size of each element to be written
 * @param nmemb number of elements to be written
 * @param userp void pointer to user data
 * @return the total size of the data written
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Fetches JSON data from a given URL using libcurl library and returns it as a JSON object.
 * @param data_url The URL to fetch JSON data from.
 * @return A JSON object containing the fetched data. If the function fails to fetch data, an empty JSON object is returned.
 * @note This function requires the libcurl library to be installed.
 * @note The returned JSON object must be deallocated manually.
 * @note This function throws an exception if there is an error parsing the fetched JSON data.
 */
Json::Value fetch_data(const std::string& data_url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, data_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    if (readBuffer.empty()) {
        std::cerr << "Failed to fetch data from " << data_url << std::endl;
        return Json::Value();
    }

    Json::Value jsonData;
    std::istringstream readStream(readBuffer);
    readStream >> jsonData;
    return jsonData;
}

/**
 * @brief Plays an alert sound from a given sound file path using the 'mpg123' command-line tool.
 * This function executes a system command to play the sound file in the background
 * and returns immediately, without waiting for the playback to complete.
 * @param sound_file The path of the sound file to be played.
 * @return None.
 * @throw None.
 *
 * @note This function requires the 'mpg123' command-line tool to be installed on the system.
 */
void play_alert_sound(const std::string& sound_file) {
    std::string cmd = "mpg123 -q " + sound_file + " &";
    std::system(cmd.c_str());
}

/**
 * @brief a GTK message dialog box with the specified title, message, and button options.
 * @param title: The title of the message dialog box.
 * @param message: The main message displayed in the message dialog box.
 * @param message_type: The type of message dialog box (error, warning, info, etc.).
 * @param buttons_type: The type of buttons to display in the message dialog box (ok, cancel, yes-no, etc.).
 * @note: The function creates a GTK message dialog box with the specified parameters and displays it to the user.
 * @note: The dialog box will have a main message and a title displayed at the top, and will include buttons based on the specified button type.
 * @note: The function will wait for the user to interact with the dialog box before returning.
 * @note: This function requires a running GTK event loop. You should call it from a GTK application context.
 */
void show_dialog(const std::string& title, const std::string& message, Gtk::MessageType message_type, Gtk::ButtonsType buttons_type) {
    auto app = Gtk::Application::create("com.example.alert");
    Gtk::MessageDialog dialog(title, false, message_type, buttons_type, true);
    dialog.set_secondary_text(message);
    dialog.signal_response().connect([&](int response_id){
        dialog.close();
    });
    app->run(dialog);
}

/**
 * @brief Continuously checks data from a specified URL for updates and triggers alert events based on changes.
 * This function continuously fetches data from a specified URL and checks it for changes at a specified interval.
 * If the data indicates a change that warrants an alert, an alert sound and a GTK message dialog box will be triggered.
 * The alert sound is played using the 'mpg123' command-line tool and runs in the background without blocking other actions.
 * The GTK message dialog box displays a warning message with the region and status information.
 * @param alert_on The path of the alert sound file to be played when an alert is triggered.
 * @param alert_off The path of the alert sound file to be played when an alert is deactivated.
 * @param data_url The URL of the data source to fetch the data from.
 * @param update_interval The time interval (in seconds) to check for updates from the data source.
 * @return None.
 * @note This function requires a running GTK event loop. You should call it from a GTK application context.
 */
void check_alerts(const std::string& alert_on, const std::string& alert_off, const std::string& data_url, int update_interval) {
    while (true) {
        Json::Value data = fetch_data(data_url);
        if (data.empty()) {
            std::cerr << "Failed to fetch data from " << data_url << std::endl;
            continue; // continue the cycle without performing other actions
        }
        std::string status = data[region].asString();

        if (!alert_active && status == "full") {
            alert_active = true;
            std::thread sound_thread( play_alert_sound, alert_on );
            sound_thread.detach();
            std::thread dialog_thread(show_dialog, "ВСІ В УКРИТТЯ!!!",
                                    "Увага! Повітряна тривога в регіоні: " + region + "!",
                                    Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
            dialog_thread.detach();
        } else if (alert_active && (status == "null" || status == "no_data")) {
            alert_active = false;
            std::thread sound_thread( play_alert_sound, alert_off );
            sound_thread.detach();
            std::thread dialog_thread(show_dialog, "МОЖНА ПОВЕРТАТИСЬ НА РОБОЧІ МІСЦЯ!",
                                    "Відбій повітряної тривоги в регіоні: " + region + "!",
                                    Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
            dialog_thread.detach();
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(update_interval));
    }
}

/**
* @brief The main entry point for the application.
* This function reads a configuration file specified as a command line argument and extracts the necessary parameters.
* Then it calls the 'check_alerts' function to monitor the alert status and play alert sounds and display dialogs if needed.
* @param argc An integer argument count of the command line arguments.
* @param argv An argument vector of the command line arguments.
* @return An integer value indicating the exit status of the program (0 for success, non-zero for failure).
* @note The configuration file must be in the JSON format and contain the following fields:
* "region": the region code to monitor
* "alert_on": the path to the sound file to play when the alert status changes to "full"
* "alert_off": the path to the sound file to play when the alert status changes from "full" to "null" or "no_data"
* "data_url": the URL of the data source to fetch the alert status from
* "update_interval": the interval in seconds between the status checks
 */
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>\n";
        return 1;
    }
    std::ifstream config_file(argv[1]);
    if (!config_file) {
        std::cerr << "Failed to open config file: " << argv[1] << "\n";
        return 1;
    }
    Json::Value config;
    config_file >> config;

    region = config["region"].asString();
    alert_on = config["alert_on"].asString();
    alert_off = config["alert_off"].asString();
    data_url = config["data_url"].asString();
    update_interval = config["update_interval"].asInt();

    check_alerts(alert_on, alert_off, data_url, update_interval);

    return 0;
}
