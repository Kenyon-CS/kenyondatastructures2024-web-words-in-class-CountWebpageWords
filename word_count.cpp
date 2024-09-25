#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <cctype>
#include <curl/curl.h>

using namespace std;

// Callback function to write data received from curl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch the content of the web page
string fetchWebPage(const string& url) {
    CURL* curl;
    CURLcode res;
    string data;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if(res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "";
        }
    }
    return data;
}

// Function to remove HTML tags
string removeHTMLTags(const string& content) {
    string result;
    bool insideTag = false;

    for (char ch : content) {
        if (ch == '<') {
            insideTag = true; // Start of an HTML tag
        } else if (ch == '>') {
            insideTag = false; // End of an HTML tag
        } else if (!insideTag) {
            result += ch; // Add characters outside of tags
        }
    }

    return result;
}

// Function to process the web page content and count words based on whitelist
void countWords(const string& content, map<string, int>& wordCount, const set<string>& whitelist) {
    stringstream ss(content);
    string word;

    while (ss >> word) {
        // Remove punctuation and convert to lowercase
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        transform(word.begin(), word.end(), word.begin(), ::tolower);

        // Only count words in the whitelist
        if (!word.empty() && whitelist.find(word) != whitelist.end()) {
            wordCount[word]++;
        }
    }
}

// Function to display the word counts in ascending order (default)
void displayWordCounts(const map<string, int>& wordCount) {
    for (const auto& pair : wordCount) {
        cout << pair.first << ": " << pair.second << endl;
    }
}

// Function to display the word counts in descending order by count
void displayWordCountsDescending(const map<string, int>& wordCount) {
    // Create a vector of pairs (word, count) to store the data
    vector<pair<string, int>> sortedWords(wordCount.begin(), wordCount.end());

    // Sort the vector by the word count in descending order
    sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
    });

    // Display the sorted word counts
    for (const auto& pair : sortedWords) {
        cout << pair.first << ": " << pair.second << endl;
    }
}

// Function to read newline-separated words from a file into a set
set<string> readWordsFromFile(const string& filename) {
    set<string> words;
    string word;

    ifstream file(filename); // Open the file

    // Check if the file was opened successfully
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return words;
    }

    // Read words line by line from the file
    while (getline(file, word)) {
        // Insert each word into the set
        if (!word.empty()) {
            words.insert(word);
        }
    }

    file.close();  // Close the file
    return words;
}

int main() {
    string url;
    cout << "Enter the URL of the web page: ";
    getline(cin, url);

    string content = fetchWebPage(url);
    if (content.empty()) {
        cerr << "Failed to fetch content from URL." << endl;
        return 1;
    }

    // Remove HTML tags from the fetched content
    string cleanContent = removeHTMLTags(content);

    // Define and fill a whitelist of words (add more words as needed)
    set<string> whitelist = readWordsFromFile("words.txt");
    
    // Word counting
    map<string, int> wordCount;
    countWords(cleanContent, wordCount, whitelist);

    cout << "\nWord counts (ascending order):" << endl;
    displayWordCounts(wordCount);

    cout << "\nWord counts (descending order by frequency):" << endl;
    displayWordCountsDescending(wordCount);

    return 0;
}
