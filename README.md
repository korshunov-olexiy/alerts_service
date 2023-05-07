# Alert System
This program continuously fetches data from a specified URL and checks it for updates at a specified interval. If the data indicates a change that warrants an alert, an alert sound and a GTK message dialog box will be triggered.

# Dependencies
This program requires the following libraries to be installed:

```
libcurl
jsoncpp
gtkmm-3.0
gstreamermm-1.0
```
In addition, the mpg123 command-line tool is required to play alert sounds.

# Installation
To install the required dependencies, use the following commands:

```
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install libjsoncpp-dev
sudo apt-get install libgtkmm-3.0-dev
sudo apt-get install libgstreamermm-1.0-dev
sudo apt-get install mpg123
```

To compile the program, run the following command:

```
g++ alert_system.cpp -o alert_system `pkg-config --cflags --libs gtkmm-3.0 gstreamermm-1.0 libcurl jsoncpp
```

Create config.json:
```
{
    "region": "Crimea",
    "alert_on": "/path/to/file/alert_on.mp3",
    "alert_off": "/pat/to/file/alert_off.mp3",
    "data_url": "https://sirens.in.ua/api/v1/",
    "update_interval": 60
}
```

where:
- region: The region to monitor for alerts. See the json object returned by https://sirens.in.ua/api/v1/
- alert_on_sound: The path to the sound file to be played when an alert is triggered.
- alert_off_sound: The path to the sound file to be played when an alert is deactivated.
- data_url: The URL of the data source to fetch the data from.
- update_interval: The time interval (in seconds) to check for updates from the data source.

# Usage
To use the program, run the following command:

```
./alert_system config.json
```

# Functionality
The program includes the following functions:

fetch_data(): Fetches JSON data from a given URL using libcurl library and returns it as a JSON object.
play_alert_sound(): Plays an alert sound from a given sound file path using the 'mpg123' command-line tool.
show_dialog(): Displays a GTK message dialog box with the specified title, message, and button options.
main(): Continuously checks data from a specified URL for updates and triggers alert events based on changes.

[Sponsor this project](https://www.buymeacoffee.com/alexkan)
