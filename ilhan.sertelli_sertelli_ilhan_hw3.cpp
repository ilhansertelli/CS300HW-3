#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include "BST.h"
#include "HashTable.h"
#include "strutils.h"
using namespace std;

struct DocumentItem {
    string documentName;
    int count;

    DocumentItem(const string & name, const int & c): documentName(name), count(c) {}
};

struct WordItem {
    string word;
    BST<string, DocumentItem> docInfo; // Document info is stored in tree

    WordItem(const string & w) : word(w) {}
};

// Parsing the query into words and creating the list of the words
vector<string> parser(string query) {
    vector<string> words;
    int idx = 0;
    while (idx < query.length()) {
        string tempWord = "";
        if (('a' <= query.at(idx) && query.at(idx) <= 'z') || ('A' <= query.at(idx) && query.at(idx) <= 'Z')) {
            while (idx < query.length() && ('a' <= query.at(idx) && query.at(idx) <= 'z') || ('A' <= query.at(idx) && query.at(idx) <= 'Z')) {
                tempWord += query.at(idx);
                idx++;
                if (idx == query.length())
                    break;
            }
        }
        else
            idx++;
        if (tempWord != "") {
            ToLower(tempWord);
            words.push_back(tempWord);
        }
    }
    return words;
}

int main() {

    string notFound;
    int numFiles, size = 101;
    vector<string> fileList; // The list of input files
    cout << "Enter number of input files: ";
    cin >> numFiles;

    // Adding the files into the list
    for (int i = 1; i <= numFiles; i++) {
        string fileName;
        cout << "Enter " << i << ". file name: ";
        cin >> fileName;
        fileList.push_back(fileName);
    }

    HashTable<string, WordItem*> table(notFound, size);
    BST<string, WordItem*> searchTree;
    for (int j = 0; j < fileList.size(); j++) {
        ifstream file;
        string word;
        file.open(fileList[j].c_str());
        if (!file.fail()) {
            // Getting all the words one by one
            while (getline(file, word)) {
                ToLower(word);
                int idx = 0;
                for (int k = 0; k < word.length(); k++) {
                    if (!(('a' <= word.at(k) && word.at(k) <= 'z') || ('A' <= word.at(k) && word.at(k) <= 'Z'))) {
                        word.at(k) = ' ';
                        StripWhite(word);
                    }
                }
                while (idx < word.length()) {
                    string newWord = "";
                    if (word[idx] != ' ') {
                        while (word[idx] != ' ' && idx < word.length()) {
                            newWord += word[idx];
                            idx++;
                        }
                        // Word Extracted
                        WordItem * item = new WordItem(newWord);
                        // Add the word to the tree if it does not exist
                        //----------------------------------------- BST OPERATION ----------------------------------------------
                        if (searchTree.find(newWord) == nullptr) {
                            DocumentItem doc(fileList[j], 1);
                            item->word = newWord;
                            item->docInfo.insert(fileList[j], doc);
                            searchTree.insert(newWord, item);
                        }
                            // If the word exists
                        else {
                            if (searchTree.find(newWord)->key == newWord) {
                                // Check if the file is already stored in the word node
                                if (searchTree.find(newWord)->value->docInfo.find(fileList[j]) == nullptr) {
                                    DocumentItem doc(fileList[j], 1);
                                    searchTree.find(newWord)->value->docInfo.insert(fileList[j], doc);
                                }
                                    // If the file is already in the word node
                                else {
                                    // Storing the count of the appearance of the word in the file
                                    int newCount = searchTree.find(newWord)->value->docInfo.find(fileList[j])->value.count;
                                    newCount++;
                                    // Removing the old count-version and adding the +1 version to the tree (updating)
                                    searchTree.find(newWord)->value->docInfo.remove(fileList[j], searchTree.find(newWord)->value->docInfo.find(fileList[j])->value);
                                    DocumentItem newDoc(fileList[j], newCount);
                                    searchTree.find(newWord)->value->docInfo.insert(fileList[j], newDoc);
                                }
                            }
                        }
                        //----------------------------------------- HASH OPERATION ----------------------------------------------

                        WordItem * item1 = new WordItem(newWord);
                        if (table.find(newWord).key == notFound) {
                            DocumentItem doc(fileList[j], 1);
                            item1->word = newWord;
                            item1->docInfo.insert(fileList[j], doc);
                            table.insert(newWord, item1);
                        }
                        else {
                            if (table.find(newWord).key == newWord) {
                                // Check if the file is already stored in the word node
                                if (table.find(newWord).value->docInfo.find(fileList[j]) == nullptr) {
                                    DocumentItem doc(fileList[j], 1);
                                    table.find(newWord).value->docInfo.insert(fileList[j], doc);
                                }
                                else {
                                    // Storing the count of the appearance of the word in the file
                                    int newCount = table.find(newWord).value->docInfo.find(fileList[j])->value.count;
                                    newCount++;
                                    // Removing the old count-version and adding the +1 version to the table (updating)
                                    table.find(newWord).value->docInfo.remove(fileList[j], table.find(newWord).value->docInfo.find(fileList[j])->value);
                                    DocumentItem newDoc(fileList[j], newCount);
                                    table.find(newWord).value->docInfo.insert(fileList[j], newDoc);
                                }
                            }
                        }
                    }
                    else
                        idx++;
                }
            }
        }
    }

    cout << endl << endl << "After preprocessing, the unique word count is " << table.getTableSize() << ". Current load ratio is " << table.getLoadFactor() << endl << endl;

    string query;
    cout << "Enter queried words in one line: ";
    cin.ignore();
    getline(cin, query);
    vector<string> parsedQuery = parser(query);

    int k = 20;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < k; i++)
    {
        if (i == 0) {
            // QueryDocuments(with BST);
            bool none = true; // True -> The query does not exist in none of the files
            for (int i = 0; i < fileList.size(); i++) {
                bool inEach = true; // True -> The query words exist in the file
                for (int j = 0; j < parsedQuery.size(); j++) {
                    // If the word already exists in the tree
                    if (searchTree.find(parsedQuery[j]) != nullptr) {
                        // Checking if the file is stored in the node
                        if (searchTree.find(parsedQuery[j])->value->docInfo.find(fileList[i]) == nullptr)
                            inEach = false;
                    } else
                        inEach = false;
                }
                if (inEach) { // It means that every word in the query exists in at least one file
                    cout << "in Document " << fileList[i] << ", ";
                    for (int k = 0; k < parsedQuery.size(); k++) {
                        if (k != parsedQuery.size() - 1)
                            cout << parsedQuery[k] << " found "
                                 << searchTree.find(parsedQuery[k])->value->docInfo.find(fileList[i])->value.count
                                 << " times, ";
                        else
                            cout << parsedQuery[k] << " found "
                                 << searchTree.find(parsedQuery[k])->value->docInfo.find(fileList[i])->value.count
                                 << " times.";
                    }
                    cout << endl;
                    none = false;
                }
            }
            if (none) { // It means that none of the file contains every word in the query
                cout << "No document contains the given query" << endl;
            }
        }
    }
    auto BSTTime = std::chrono::duration_cast<std::chrono::nanoseconds>
            (std::chrono::high_resolution_clock::now() - start);

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < k; i++)
    {
        // QueryDocuments (with hashtable);
        if (i == 0) {
            bool none = true; // True -> The query does not exist in none of the files
            for (int i = 0; i < fileList.size(); i++) {
                bool inEach = true; // True -> The query words exist in the file
                for (int j = 0; j < parsedQuery.size(); j++) {
                    // If the word already exists in the table
                    if (table.find(parsedQuery[j]).key != notFound) {
                        // Checking if the file is stored in the node
                        if (table.find(parsedQuery[j]).value->docInfo.find(fileList[i]) == nullptr)
                            inEach = false;
                    } else
                        inEach = false;
                }
                if (inEach) { // It means that every word in the query exists in at least one file
                    cout << "in Document " << fileList[i] << ", ";
                    for (int k = 0; k < parsedQuery.size(); k++) {
                        if (k != parsedQuery.size() - 1)
                            cout << parsedQuery[k] << " found "
                                 << table.find(parsedQuery[k]).value->docInfo.find(fileList[i])->value.count
                                 << " times, ";
                        else
                            cout << parsedQuery[k] << " found "
                                 << table.find(parsedQuery[k]).value->docInfo.find(fileList[i])->value.count
                                 << " times.";
                    }
                    cout << endl;
                    none = false;
                }
            }
            if (none) { // It means that none of the file contains every word in the query
                cout << "No document contains the given query" << endl;
            }
        }
    }
    auto HTTime = std::chrono::duration_cast<std::chrono::nanoseconds>
            (std::chrono::high_resolution_clock::now() - start);
    cout << "\nTime: " << BSTTime.count() / k << "\n";
    cout << "\nTime: " << HTTime.count() / k << "\n";
    cout << "Speed Up: " << double(BSTTime.count() / k) / double(HTTime.count() / k);


    return 0;
}
